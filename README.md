runtime_scanner can be built with 'make'
followed by 'make clean' to remove the remaining object files.

The program inputs are:

    runtime_scanner [-p project_path] [g grep_file_path] [-i] [-h] [-o] [-t]

These are expanded upon using the --help or -? flags

Some sample uses:

    ./runtime_scanner -i -h -o -p /android-7.0.0_r1 -g grep.txt
    
    grep -rn --include=\*.java "\.exec(" . | runtime_scanner -p .

The original application for the program is for Google Android's AOSP. Instructions
on how to download that are given at https://source.android.com/setup/downloading.
Note that the download is between 50-75GB depending on the branch. The program is
designed for Linux so it should be built with that consideration in mind.
