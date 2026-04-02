# PPMVIEWER

A simple image viewer for .ppm files.

## Usage

You can either set the executable as the default program for .ppm file extension, or open them from CLI by providing the path to the file:

ppm [file-path]

#### Example:

```bash
ppm ./mogged.ppm
```

#### NOTE: When compiled with -mwindows flag the program will have no stdout output when called from CLI 

## Quick start

#### Prerequisites: You need to have gcc installed and added to PATH environment variable.

Build from source.

```bash
./build.bat
```

Add the executable to PATH environment variable or just set it as a default program for .ppm files