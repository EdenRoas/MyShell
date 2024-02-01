# MyShell

# What is this project?

This project is a shell program

# The way we made our project:
Basiclly this is an infinite loop reciving commands from the user. 

If the command is in our "command arsenal" we will execute it, if not we will send it to a regular bash and try to commit it.

# Command arsenal

*quit* - exits the shell.

*echo* - prints what ever comes after the ECHO.

*cd* - try to enter the reposetory after the CD command (MUST BE INSIDE THE CURRET REPOSETORY).

*prompt* - change the Shell name.

*!!* - does the last command commited.

*$?* - prints last command's status.

*$x = Eden* - variable decleration.

*read x | Eden* - variable decleration using scanning input from user.

# Examples from my terminal:
* @EdenRoas ➜ /workspaces/MyShell (main) $ if date | grep Fri
> then
> echo "Shabat Shalom"
> else
> echo "Hard way to go"
> fi
Hard way to go
*@EdenRoas ➜ /workspaces/MyShell (main) $ ./myshell
hello: date >> myfile
hello: cat myfile
cat: myfile: Permission denied
hello: date –u >> myfile
date: invalid date ‘–u’
hello: cat myfile
cat: myfile: Permission denied
hello: wc -l < myfile
wc: 'standard input': Bad file descriptor
0
wc: -: Bad file descriptor
hello: prompt = hi
Changed prompt name to hi 
hi  mkdir mydir
hi  cd mydir
chdir() to mydir
hi  pwd
/workspaces/MyShell/mydir
hi  touch file1 file2 file3
hi  ls
file1  file2  file3
hi  !!
file1  file2  file3
hi  echo abc xyz
abc xyz 
hi  ls
file1  file2  file3
hi  echo $?
0
hi  ls no_such_file
ls: cannot access 'no_such_file': No such file or directory
hi  echo $?
512
hi  ls no_such_file 2> file
hi  Control-C
> hi:  touch colors.txt
hi:  cat > colors.txt
blue
black
red
red
green 
blue
green
red
red
blue
hi:  cat colors.txt
blue
black
red
red
green 
blue
green
red
red
blue
hi:  cat colors.txt | cat | cat | cat
blue
black
red
red
green 
blue
green
red
red
blue
hi:  sort colors.txt | uniq –c | sort –r | head -3
uniq: –c: No such file or directory
sort: cannot read: –r: No such file or directory
hi:  quit
Quit button detected, Thanks for using hello Shell




# How to use our project:

Write in a bash envioremnt "make all". 

Please run ./myshell

To clean the resources created write "make clean".
### Authors
Eden Roas

Tal Tosiano
# Hope you find good usuage of this project!
