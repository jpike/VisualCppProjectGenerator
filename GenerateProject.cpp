#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

/// The standard path separator on Windows, as used within Visual Studio project files.
const std::string PATH_SEPARATOR = "\\";
/// The file extension for C++ source files.
const std::string CPP_FILE_EXTENSION = ".cpp";
/// The file extension for a Visual Studio solution file.
const std::string SOLUTION_FILE_EXTENSION = ".sln";
/// The file extension for a Visual Studio project file.
const std::string PROJECT_FILE_EXTENSION = ".vcxproj";
/// The file extension for a Visual Studio project filters file.
const std::string PROJECT_FILTERS_FILE_EXTENSION = ".vcxproj.filters";

/// A file on the file system.
class File
{
public:
    // METHODS.
    /// Constructor.
    /// @param[in]  relative_folder_path - The relative path to the folder containing the file.
    ///     This path may be relative to anything, but mixing paths relative to different
    ///     things is not recommended in the same program.
    /// @param[in]  name - The filename.
    explicit File(const std::string& relative_folder_path, const std::string& name) :
        RelativeFolderPath(relative_folder_path),
        RelativePath(),
        Name(name)
    {
        // POPULATE THE RELATIVE FILEPATH.
        if (relative_folder_path.empty())
        {
            // Without a relative folder path, the relative filepath is just the filename.
            RelativePath = Name;
        }
        else
        {
            // The full relative filepath includes the relative folder path.
            RelativePath = RelativeFolderPath + PATH_SEPARATOR + Name;
        }
    }

    /// Gets the file extension with the leading dot.
    /// Assumes the file extension occurs after the last dot in the filename.
    /// @return The file's extension with its leading dot.
    std::string ExtensionWithLeadingDot() const
    {
        // FIND THE LAST DOT IN THE FILENAME.
        const char FILE_EXTENSION_SEPARATOR = '.';
        std::string::size_type last_dot = Name.find_last_of(FILE_EXTENSION_SEPARATOR);
        bool extension_separator_found = (std::string::npos != last_dot);
        if (extension_separator_found)
        {
            // RETURN THE FILE EXTENSION WITH ITS LEADING DOT.
            std::string extension = Name.substr(last_dot);
            return extension;
        }
        else
        {
            // Without a leading dot, assume there is no file extension.
            const std::string NO_EXTENSION = "";
            return NO_EXTENSION;
        }
    }
    
    // MEMBER VARIABLES.
    /// The relative path to the folder containing the file.
    std::string RelativeFolderPath;
    /// The relative path to the file.
    std::string RelativePath;
    /// The name of the file, including any extension.
    std::string Name;
};

/// A folder on the file system.
class Folder
{
public:
    // METHODS.
    /// Gets the folder at the specified path, with its lists of files and subfolders populated.
    /// @param[in]  relative_path - The relative path to the folder.
    ///     This path may be relative to anything, but mixing paths relative to different
    ///     things is not recommended in the same program.
    /// @return The folder at the specified path, with its lists of files and subfolders populated
    ///     (assuming any are found).
    static Folder Get(const std::string& relative_path)
    {
        Folder folder(relative_path);
        
        // FIND ALL FILES AND FOLDERS WITHIN THE CURRENT FOLDER.
        const std::string WILDCARD = "*";
        std::string find_all_files_and_folders = relative_path + PATH_SEPARATOR + WILDCARD;
        
        // This gets the initial file, which should be the current folder.
        // Error handling could be performed here, but given the context of this program,
        // it isn't quite worth it yet to add more complicated error handling.
        // The only side-effects of this failing without error handling is that
        // a folder is returned without any subfolders or files.
        WIN32_FIND_DATA file_data;
        HANDLE search_handle = FindFirstFile(find_all_files_and_folders.c_str(), &file_data);
        
        while (FindNextFile(search_handle, &file_data))
        {
            // ADD THE CURRENT "FILE" TO THE APPROPRIATE LIST DEPENDING ON WHETHER IT IS A FOLDER OR NOT.
            const unsigned int FILE_ATTRIBUTE_NOT_SET = 0;
            bool is_directory = (FILE_ATTRIBUTE_NOT_SET != (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
            if (is_directory)
            {
                // CHECK IF THE CURRENT FILE IS FOR THE CURRENT OR PARENT DIRECTORIES.
                // These entries may be found by the file finding functions.  Since we don't want to
                // include them in the list of subfolders, they should be skipped.
                bool is_current_directory = (std::string(file_data.cFileName) == ".");
                bool is_parent_directory = (std::string(file_data.cFileName) == "..");
                bool is_abbreviated_directory = (is_current_directory || is_parent_directory);
                if (is_abbreviated_directory)
                {
                    continue;
                }
                
                // GET AND ADD THE POPULATED SUBFOLDER.
                std::string subfolder_path = relative_path + PATH_SEPARATOR + file_data.cFileName;
                Folder subfolder = Get(subfolder_path);
                folder.Subfolders.push_back(subfolder);
            }
            else
            {
                // ADD THE FILE.
                folder.Files.emplace_back(folder.RelativePath, file_data.cFileName);
            }
        }
        
        return folder;
    }

    /// Constructor.  The lists of files and subfolders won't be populated.
    /// @param[in]  relative_path - The relative path to the folder.
    ///     This path may be relative to anything, but mixing paths relative to different
    ///     things is not recommended in the same program.
    explicit Folder(const std::string& relative_path) :
        RelativePath(relative_path),
        Subfolders(),
        Files()
    {}

    /// Gets all header files in this folder and all subfolders.
    /// @return All header files in this folder.
    std::vector<File> GetHeaderFiles() const
    {
        std::vector<File> header_files;

        // GET ALL HEADER FILES DIRECTLY IN THIS FOLDER.
        for (const auto& file : Files)
        {
            // INCLUDE THE CURRENT FILE IF IT'S A HEADER FILE.
            const std::string HEADER_FILE_EXTENSION = ".h";
            std::string extension = file.ExtensionWithLeadingDot();
            bool is_header_file = (HEADER_FILE_EXTENSION == extension);
            if (is_header_file)
            {
                header_files.push_back(file);
            }
        }
        
        // GET ALL HEADER FILES IN SUBFOLDERS.
        for (const auto& subfolder : Subfolders)
        {
            // INCLUDE HEADER FILES IN THIS SUBFOLDER.
            std::vector<File> subfolder_header_files = subfolder.GetHeaderFiles();
            header_files.insert(
                header_files.end(),
                subfolder_header_files.cbegin(),
                subfolder_header_files.cend());
        }
        
        return header_files;
    }
    
    /// Gets all .cpp files in this folder and all subfolders.
    /// @return All .cpp files in this folder.
    std::vector<File> GetCppFiles() const
    {
        std::vector<File> cpp_files;

        // GET ALL CPP FILES DIRECTLY IN THIS FOLDER.
        for (const auto& file : Files)
        {
            // INCLUDE THE CURRENT FILE IF IT'S A CPP FILE.
            std::string extension = file.ExtensionWithLeadingDot();
            bool is_cpp_file = (CPP_FILE_EXTENSION == extension);
            if (is_cpp_file)
            {
                cpp_files.push_back(file);
            }
        }
        
        // GET ALL CPP FILES IN SUBFOLDERS.
        for (const auto& subfolder : Subfolders)
        {
            // INCLUDE CPP FILES IN THIS SUBFOLDER.
            std::vector<File> subfolder_cpp_files = subfolder.GetCppFiles();
            cpp_files.insert(
                cpp_files.end(),
                subfolder_cpp_files.cbegin(),
                subfolder_cpp_files.cend());
        }
        
        return cpp_files;
    }

    /// Gets all folders in this folder and including this folder.
    /// @return This folder and all subfolders.
    std::vector<Folder> GetAllFolders() const
    {
        // INCLUDETHIS FOLDER.
        std::vector<Folder> all_folders = { *this };
        
        // INCLUDE ALL SUBFOLDERS.
        for (const auto& subfolder : Subfolders)
        {
            // INCLUDE ALL SUBFOLDERS OF THIS SUBFOLDER.
            std::vector<Folder> subfolders = subfolder.GetAllFolders();
            all_folders.insert(
                all_folders.end(),
                subfolders.cbegin(),
                subfolders.cend());
        }
        
        return all_folders;
    }
    
    /// Debug printing for a folder.
    void DebugPrint() const
    {
        // PRINT THE FOLDER'S PATH.
        std::cout << "RelativePath: " << RelativePath << std::endl;
        
        // PRINT ALL FILES IN THE ROOT OF THIS FOLDER.
        std::cout << "Files: " << std::endl;
        for (const auto& file : Files)
        {
            std::cout << "\t" << file.RelativePath << std::endl;
        }
        
        // PRINT ALL SUBFOLDERS.
        std::cout << "Subfolders: " << std::endl;
        for (const auto& subfolder : Subfolders)
        {
            subfolder.DebugPrint();
        }
    }
    
    /// MEMBER VARIABLES.
    /// The relative path to the folder.
    std::string RelativePath;
    /// A list of subfolders within this folder.
    std::vector<Folder> Subfolders;
    /// A list of files in the direct root of this folder.
    std::vector<File> Files;
};

/// A Visual Studio solution file.
class SolutionFile
{
public:
    /// Writes a Visual Studio solution file.
    /// @param[in]  project_name - The project name for the solution file being generated.
    /// @param[in,out]  file - The file to write to.
    static void Write(const std::string& project_name, std::ofstream& file)
    {
        // Re-using the same unique IDs across all solution files does not seem to cause any problems.
        file
            << "Microsoft Visual Studio Solution File, Format Version 12.00" << std::endl
            << "# Visual Studio 2013" << std::endl
            << "VisualStudioVersion = 12.0.31101.0" << std::endl
            << "MinimumVisualStudioVersion = 10.0.40219.1" << std::endl
            << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << project_name << "\", \"" << project_name << ".vcxproj\", \"{46D99A72-17AF-4E62-809F-EECB637F6EE1}" << std::endl
            << "EndProject" << std::endl
            << "Global" << std::endl
            << "    GlobalSection(SolutionConfigurationPlatforms) = preSolution" << std::endl
            << "        CommandLineBuild|Win32 = CommandLineBuild|Win32" << std::endl
            << "        Debug|Win32 = Debug|Win32" << std::endl
            << "        Release|Win32 = Release|Win32" << std::endl
            << "    EndGlobalSection" << std::endl
            << "    GlobalSection(ProjectConfigurationPlatforms) = postSolution" << std::endl
            << "        {46D99A72-17AF-4E62-809F-EECB637F6EE1}.CommandLineBuild|Win32.ActiveCfg = Release|Win32" << std::endl
            << "        {46D99A72-17AF-4E62-809F-EECB637F6EE1}.CommandLineBuild|Win32.Build.0 = Release|Win32" << std::endl
            << "        {46D99A72-17AF-4E62-809F-EECB637F6EE1}.Debug|Win32.ActiveCfg = Debug|Win32" << std::endl
            << "        {46D99A72-17AF-4E62-809F-EECB637F6EE1}.Debug|Win32.Build.0 = Debug|Win32" << std::endl
            << "        {46D99A72-17AF-4E62-809F-EECB637F6EE1}.Release|Win32.ActiveCfg = Release|Win32" << std::endl
            << "        {46D99A72-17AF-4E62-809F-EECB637F6EE1}.Release|Win32.Build.0 = Release|Win32" << std::endl
            << "    EndGlobalSection" << std::endl
            << "    GlobalSection(SolutionProperties) = preSolution" << std::endl
            << "        HideSolutionNode = FALSE" << std::endl
            << "    EndGlobalSection" << std::endl
            << "EndGlobal" << std::endl;
    }
};

/// A Visual Studio project file.
class ProjectFile
{
public:
    /// Writes a Visual Studio project file.
    /// @param[in]  project_name - The project name for the project file being generated.
    /// @param[in]  header_files - The header files to include in the project file.
    /// @param[in]  cpp_files - The .cpp files to include in the project file.
    /// @param[in,out]  file - The file to write to.
    static void Write(
        const std::string& project_name, 
        const std::vector<File>& header_files, 
        const std::vector<File>& cpp_files, 
        std::ofstream& file)
    {
        // WRITE THE PART OF THE PROJECT FILE BEFORE THE HEADER FILES.
        file
            << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl
            << "<Project DefaultTargets=\"Build\" ToolsVersion=\"12.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl
            << "  <ItemGroup Label=\"ProjectConfigurations\">" << std::endl
            << "    <ProjectConfiguration Include=\"Debug|Win32\">" << std::endl
            << "      <Configuration>Debug</Configuration>" << std::endl
            << "      <Platform>Win32</Platform>" << std::endl
            << "    </ProjectConfiguration>" << std::endl
            << "    <ProjectConfiguration Include=\"Release|Win32\">" << std::endl
            << "      <Configuration>Release</Configuration>" << std::endl
            << "      <Platform>Win32</Platform>" << std::endl
            << "    </ProjectConfiguration>" << std::endl
            << "  </ItemGroup>" << std::endl
            << "  <ItemGroup>" << std::endl;
            
        // WRITE THE INCLUSIONS FOR THE HEADER FILES.
        for (const auto& header_file : header_files)
        {
            file << "    <ClInclude Include=\"" << header_file.RelativePath << "\" />"  << std::endl;
        }
        
        file << "  </ItemGroup>" << std::endl;
        
        // WRITE THE INCLUSIONS FOR THE CPP FILES.
        file << "  <ItemGroup>" << std::endl;
        for (const auto& cpp_file : cpp_files)
        {
            file << "    <ClCompile Include=\"" << cpp_file.RelativePath << "\" />" << std::endl;
        }
            
        // WRITE THE REMAINDER OF THE PROJECT FILE.
        file
            << "  </ItemGroup>" << std::endl
            << "  <ItemGroup>" << std::endl
            << "    <None Include=\"build.bat\" />" << std::endl
            << "  </ItemGroup>" << std::endl
            << "  <PropertyGroup Label=\"Globals\">" << std::endl
            // Re-using the same unique IDs across all project files does not seem to cause any problems.
            << "    <ProjectGuid>{46D99A72-17AF-4E62-809F-EECB637F6EE1}</ProjectGuid>" << std::endl
            << "    <Keyword>MakeFileProj</Keyword>" << std::endl
            << "    <ProjectName>" << project_name << "</ProjectName>" << std::endl
            << "  </PropertyGroup>" << std::endl
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />" << std::endl
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\" Label=\"Configuration\">" << std::endl
            << "    <ConfigurationType>Makefile</ConfigurationType>" << std::endl
            << "    <UseDebugLibraries>true</UseDebugLibraries>" << std::endl
            << "    <PlatformToolset>v120</PlatformToolset>" << std::endl
            << "  </PropertyGroup>" << std::endl
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\" Label=\"Configuration\">" << std::endl
            << "    <ConfigurationType>Makefile</ConfigurationType>" << std::endl
            << "    <UseDebugLibraries>false</UseDebugLibraries>" << std::endl
            << "    <PlatformToolset>v120</PlatformToolset>" << std::endl
            << "  </PropertyGroup>" << std::endl
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />" << std::endl
            << "  <ImportGroup Label=\"ExtensionSettings\">" << std::endl
            << "  </ImportGroup>" << std::endl
            << "  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">" << std::endl
            << "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />" << std::endl
            << "  </ImportGroup>" << std::endl
            << "  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">" << std::endl
            << "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />" << std::endl
            << "  </ImportGroup>" << std::endl
            << "  <PropertyGroup Label=\"UserMacros\" />" << std::endl
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">" << std::endl
            << "    <NMakeBuildCommandLine>build.bat</NMakeBuildCommandLine>" << std::endl
            << "    <NMakeOutput>build\\" << project_name << ".exe</NMakeOutput>" << std::endl
            << "    <NMakePreprocessorDefinitions>WIN32;_DEBUG;$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>" << std::endl
            << "    <OutDir>build\\</OutDir>" << std::endl
            << "    <IntDir>build\\</IntDir>" << std::endl
            << "  </PropertyGroup>" << std::endl
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">" << std::endl
            << "    <NMakeBuildCommandLine>build.bat</NMakeBuildCommandLine>" << std::endl
            << "    <NMakeOutput>build\\" << project_name << ".exe</NMakeOutput>" << std::endl
            << "    <NMakePreprocessorDefinitions>WIN32;NDEBUG;$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>" << std::endl
            << "    <OutDir>build\\</OutDir>" << std::endl
            << "    <IntDir>build\\</IntDir>" << std::endl
            << "  </PropertyGroup>" << std::endl
            << "  <ItemDefinitionGroup>" << std::endl
            << "  </ItemDefinitionGroup>" << std::endl
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />" << std::endl
            << "  <ImportGroup Label=\"ExtensionTargets\">" << std::endl
            << "  </ImportGroup>" << std::endl
            << "</Project>" << std::endl;
    }
};

/// A Visual Studio project filters file.
class ProjectFiltersFile
{
public:
    /// Writes a Visual Studio project filters file.
    /// @param[in]  header_files - The header files to include in the project filters file.
    /// @param[in]  cpp_files - The .cpp files to include in the project filters file.
    /// @param[in]  folders - The folders to include as filters in file.
    /// @param[in,out]  file - The file to write to.
    static void Write(
        const std::vector<File>& header_files, 
        const std::vector<File>& cpp_files, 
        const std::vector<Folder>& folders, 
        std::ofstream& file)
    {
        // WRITE THE PART OF THE FILE BEFORE THE CPP FILES.
        file
            << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl
            << "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl
            << "  <ItemGroup>" << std::endl;
            
        // WRITE THE CPP FILES.
        for (const auto& cpp_file : cpp_files)
        {
            file
                << "    <ClCompile Include=\"" << cpp_file.RelativePath << "\">" << std::endl
                << "      <Filter>" << cpp_file.RelativeFolderPath << "</Filter>" << std::endl
                << "    </ClCompile>" << std::endl;
        }
            
        // WRITE THE PART OF THE FILE BETWEEN THE CPP FILES AND FOLDER FILTERS.
        file
            << "  </ItemGroup>" << std::endl
            << "  <ItemGroup>" << std::endl
            << "    <None Include=\"build.bat\" />" << std::endl
            << "  </ItemGroup>" << std::endl
            << "  <ItemGroup>" << std::endl;
            
        // WRITE THE FOLDER FILTERS.
        for (const auto& folder : folders)
        {
            // Re-using the same unique IDs here does not seem to cause any problems.
            file
                << "    <Filter Include=\"" << folder.RelativePath << "\">" << std::endl
                << "      <UniqueIdentifier>{96873809-db68-49b8-8a4b-a40a3c3972f6}</UniqueIdentifier>" << std::endl
                << "    </Filter>" << std::endl;
        }
        
        file
            << "  </ItemGroup>" << std::endl
            << "  <ItemGroup>" << std::endl;
            
        // WRITE THE HEADER FILES.
        for (const auto& header_file : header_files)
        {
            file
                << "    <ClInclude Include=\"" << header_file.RelativePath << "\">" << std::endl
                << "      <Filter>" << header_file.RelativeFolderPath << "</Filter>" << std::endl
                << "    </ClInclude>" << std::endl;
        }
         
        // WRITE THE REMAINDER OF THE FILE.
        file
            << "  </ItemGroup>" << std::endl
            << "</Project>" << std::endl;
    }
};

/// A basic build script as a batch file for a project.
class BuildScriptBatchFile
{
public:
    /// Writes the build script batch file.
    /// @param[in]  project_cpp_filename - The filename of .cpp file to compile to build the entire project.
    ///     The file is used as the main file for a "unity" or "single translation unit" build.
    /// @param[in]  code_folder - The folder containing all code files for the project.
    ///     It's path will be added as an include path.
    /// @param[in,out]  file - The file to write to.
    static void Write(const std::string& project_cpp_filename, const Folder& code_folder, std::ofstream& file)
    {
        file
            << "@ECHO off" << std::endl
            << std::endl
            << "REM PUT THE COMPILER IN THE PATH." << std::endl
            << "REM This isn't necessary and may cause problems if this file is run repeatedly in a command prompt." << std::endl
            << "REM Remove it if you'd prefer to take care of this in some other way." << std::endl
            << "REM Change the path if you'd prefer to use a different version of the Visual Studio compiler." << std::endl
            << "CALL \"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat\" x64" << std::endl
            << std::endl
            << "REM MOVE INTO THE BUILD DIRECTORY." << std::endl
            << "IF NOT EXIST \"build\" MKDIR \"build\"" << std::endl
            << "PUSHD \"build\"" << std::endl
            << std::endl
            << "    REM BUILD THE PROGRAM." << std::endl
            << "    REM See https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx for compiler options." << std::endl
            << "    REM The compiler options listed here are just one set of options and definitively don't support much variability." << std::endl
            << "    REM Support for more variability in this build script may be added later, but feel free to not use this basic template and" << std::endl
            << "    REM just create your own build script as needed." << std::endl
            << "    REM /Zi - debug info" << std::endl
            << "    REM /EHa - The exception-handling model that catches both asynchronous (structured) and synchronous (C++) exceptions." << std::endl
            << "    REM /WX - All warnings as errors" << std::endl
            << "    REM /W4 - Warning level 4" << std::endl
            << "    REM /MTd - Static linking with Visual C++ lib." << std::endl
            << "    REM /I - Additional include directories." << std::endl
            << "    REM user32.lib and gdi32.lib - Basic Windows functions.  Remove if not needed." << std::endl
            << "    cl.exe /Zi /EHa /WX /W4 /MTd \"..\\" << project_cpp_filename << "\" /I \"..\\" << code_folder.RelativePath << "\" user32.lib gdi32.lib" << std::endl
            << "" << std::endl
            << "POPD" << std::endl
            << std::endl
            << "@ECHO ON" << std::endl;
    }
};

/// The entry point for the Visual Studio project file generator.  The goal of this program is to make it
/// easy to generate Visual Studio project files, given a folder of code files, that allows building
/// a project using a simple build.bat script for a "unity" or "single translation unit" build.
///
/// Assuming this was compiled using the accompanying build script, the program should be run as follows:
///     GenerateProject.exe <ProjectName> <CodeFolderRelativePath>
///
/// This program will then generate the following files in the current folder:
/// - ProjectName.sln - A Visual Studio solution file containing the generated project file.
/// - ProjectName.vcxproj - A Visual Studio project file containing all .h and .cpp files in the code folder,
///     along with the build.bat script generated in the current folder that is used to build the project.
/// - ProjectName.vcxproj.filters - A Visual Studio project filters file containing the files in the project file,
///     along with the build.bat script.  Filters are added according to the folder hierarchy in the code folder.
/// - build.bat - A basic build.bat script for building the project by building a "ProjectName.cpp" file.
///     The code folder will be added as an additional include directory.  This is one of the most incomplete parts
///     of this program so far.  It doesn't support a wide variety of options, so you'll likely need to
///     make modifications (or not use it altogether).  See the generated file (or this source code) for details.
///     IMPORTANT: THIS WILL OVERWRITE ANY BUILD.BAT FILE IN THE CURRENT DIRECTORY, SO MAKE SURE YOU DON'T
///     USE THIS PROGRAM IF YOU HAVE A CUSTOM BUILD.BAT FILE!
///
/// Note that this program is still in its very early stages, and there is very little need for it
/// to be super robust or feature rich.  The goal was to just get a program working to get the bulk
/// of some mundane work taken care of.  Updates will be made as desired and as time permits.
///
/// @param[in]  argument_count - The number of command line arguments.
/// @param[in]  arguments - The command line arguments.
/// @return 0 if the program completed successfully; another value if an error occurred.
int main(int argument_count, char* arguments[])
{
    // MAKE SURE THE REQUIRED COMMAND LINE ARGUMENTS WERE PROVIDED.
    // The first argument should be the program name/path.
    const unsigned int EXPECTED_ARGUMENT_COUNT = 3;
    bool expected_arguments_provided = (EXPECTED_ARGUMENT_COUNT == argument_count);
    if (!expected_arguments_provided)
    {
        std::cerr << "Missing command line arguments!  Usage: "<< std::endl;
        std::cerr << "\t GenerateProject.exe <ProjectName> <CodeFolderRelativePath>" << std::endl;
    }
    
    // READ THE COMMAND LINE ARGUMENTS.
    const unsigned int PROJECT_NAME_COMMAND_LINE_ARGUMENT_INDEX = 1;
    std::string project_name = arguments[PROJECT_NAME_COMMAND_LINE_ARGUMENT_INDEX];
    
    const unsigned int CODE_FOLDER_PATH_COMMAND_LINE_ARGUMENT_INDEX = 2;
    std::string code_folder_path = arguments[CODE_FOLDER_PATH_COMMAND_LINE_ARGUMENT_INDEX];
    
    // GET THE CODE FOLDER.
    Folder code_folder = Folder::Get(code_folder_path);
    code_folder.DebugPrint();
    
    // WRITE THE SOLUTION FILE.
    std::string solution_filename = project_name + SOLUTION_FILE_EXTENSION;
    std::ofstream solution_file(solution_filename);
    SolutionFile::Write(project_name, solution_file);
    solution_file.close();
    
    // GET THE CODE FILES FOR THE PROJECT.
    std::vector<File> header_files = code_folder.GetHeaderFiles();
    std::vector<File> cpp_files = code_folder.GetCppFiles();
    
    // Include the main CPP file for the project for the build script.
    // There may be a better way to handle this, but that hasn't been
    // too critical to think about at this stage of development.
    const std::string IN_ROOT_CODE_FOLDER = "";
    std::string project_cpp_filename = project_name + CPP_FILE_EXTENSION;
    cpp_files.emplace_back(IN_ROOT_CODE_FOLDER, project_cpp_filename);
    
    // WRITE THE PROJECT FILE.
    std::string project_filename = project_name + PROJECT_FILE_EXTENSION;
    std::ofstream project_file(project_filename);
    ProjectFile::Write(project_name, header_files, cpp_files, project_file);
    project_file.close();
    
    // WRITE THE PROJECT FILTERS FILE.
    std::vector<Folder> code_folders = code_folder.GetAllFolders();
    std::string project_filters_filename = project_name + PROJECT_FILTERS_FILE_EXTENSION;
    std::ofstream project_filters_file(project_filters_filename);
    ProjectFiltersFile::Write(header_files, cpp_files, code_folders, project_filters_file);
    project_filters_file.close();
    
    // WRITE THE BUILD SCRIPT FILE.
    const std::string BUILD_SCRIPT_FILENAME = "build.bat";
    std::ofstream build_script_file(BUILD_SCRIPT_FILENAME);
    BuildScriptBatchFile::Write(project_cpp_filename, code_folder, build_script_file);
    build_script_file.close();

    return EXIT_SUCCESS;
}
