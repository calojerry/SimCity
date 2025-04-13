***************************************************************************
                             SIMCITY SIMULATION
***************************************************************************

===========================================================================
|                              TEAM 1 MEMBERS                              |
===========================================================================
|                            * Avery Albrecht                              |
|                            * Chet Black                                  | 
|                            * Dayveon Brown                               |
|                            * Calogero Gonzales                           |
===========================================================================

=============================================================================
|                        INSTRUCTIONS FOR COMPILING                         |
=============================================================================

1. Download the project files and place them in the appropriate directory.

2. Navigate to the project directory in your terminal:
    ```
    cd <file_path>
    ```

3. Compile and run the program by running:
    ```
    make run
    ```

4. After running the program, clean up object files by running:
    ```
    make clean
    ```

=============================================================================
|                              IMPORTANT NOTES                              |
=============================================================================

- Compilation Flags:
  - `CFLAGS = -Wall -std=c++17`
  
- File Placement:
  - Make sure that both the configuration file and region layout file are 
    located in the same directory (preferably with the program files). 
    This setup is **essential** for the program to locate the files correctly.

- Configuration File Path:
  - When prompted, provide the full path to the configuration file, 
    including the file name. Use correct syntax and nomenclature for the path 
    to be recognized.
    - Ex: When the program prompts with "Enter the path for the configuration file: ",
      an appropriate input might be:
      ```
      /home/files/SimCity/config.txt
      ```

***************************************************************************
