# Overview
This is a simple program designed to generate Visual Studio solution/project files given a source code folder.
A build.bat script is also created to assist in actually building the source code.
See http://www.jacobpike.com/blog/2016/07/29/on-c-build-systems/ for more elaboration on the intent behind this project.

Only Windows is supported since Visual Studio is Windows-only.  The source code also uses some of the Windows API,
and the build.bat file is obviously Windows-only.

This entire project is put in the public domain, per the LICENSE.txt file.

# Building the Program
The build.bat file can be used to compile the single .cpp into the actual program.  It currently assumes Visual Studio 2013,
and currently only creates a minimal debug build.  You can tweak the build.bat as desired, but it hasn't been tested in
other environments.

# Running the Program
The documentation below largely comes from the documentation for main() in GenerateProject.cpp and is largely copied here for convenience.

Assuming you have a folder with source code you'd like to generate Visual Studio project files for, you can run the program
as follows:

    GenerateProject.exe <ProjectName> <CodeFolderRelativePath>
    
This program will then generate the following files in the current folder:
* ProjectName.sln - A Visual Studio solution file containing the generated project file.
* ProjectName.vcxproj - A Visual Studio project file containing all .h and .cpp files in the code folder,
    along with the build.bat script generated in the current folder that is used to build the project.
* ProjectName.vcxproj.filters - A Visual Studio project filters file containing the files in the project file,
    along with the build.bat script.  Filters are added according to the folder hierarchy in the code folder.
* build.bat - A basic build.bat script for building the project by building a "ProjectName.cpp" file.
    This "ProjectName.cpp" file is intended to be a separate file in the current folder that solely acts
    as a "unity" or "single translation unit" build file - i.e. including all of the other .cpp files you want to build.
    The code folder will be added as an additional include directory.  This is one of the most incomplete parts
    of this program so far.  It doesn't support a wide variety of options, so you'll likely need to
    make modifications (or not use it altogether).  See the generated file (or this source code) for details.
    IMPORTANT: THIS WILL OVERWRITE ANY BUILD.BAT FILE IN THE CURRENT DIRECTORY, SO MAKE SURE YOU DON'T
    USE THIS PROGRAM IF YOU HAVE A CUSTOM BUILD.BAT FILE!

# Limitations
The program is in an extremely early state.  Therefore, it isn't very feature rich and may not be very robust
(only tested against 1 candidate project so far).  Some specific limitations include:
* Only supports Visual Studio 2013.
* Doesn't support advanced options for the project files - the files pretty much do nothing more besides include
    the code files and call the build.bat script.
* The generated build.bat script only supports an extremely basic debug build, and the program will likely
    overwrite any existing build.bat script in the entire folder (so make sure to back yours up if you
    have one!).  Any build.bat script generated will likely need modifications for any project except
    for some very basic debug/internal programs.
