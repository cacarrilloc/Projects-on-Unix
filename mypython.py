#!/usr/bin/python
#
# CARLOS CARRILLO-CALDERON
# PROGRAM 5
# CS_344-OSU
# 06/29/2016

# DESCRIPTION: This script creates 3 different files, which remain after the script finishes executing. Each of these 3 files contain exactly 10 random characters from the lowercase alphabet, with no spaces (i.e. "hoehdgwkdq"). The final character of each file is a newline character. Then, the contents of the 3 files created is printed out on screen. Finally, the script prints out two random integers (whose range is from 1 to 42, inclusive), and print out the product of these two random numbers.

# Random function in python
import random;
# String function in python
import string;

print "\n***********************************************"
print "\nTHIS IS THE CONTENT OF THE 3 FILES CREATED:"

##########################################################################
# CREATE AND DISPLAY FILE 1
file1 = open("file1.txt", "wb") # Creates or overwrite file 1
for index in range(0,10):       # For loop with 10 iterations
    # Get a random lowercase letter from the alphabet
    ramdonLetter1 = random.choice(string.ascii_lowercase);
    file1.write(ramdonLetter1); #write random letter on the file
file1.write("\n"); # Make final character a newline character.
file1.close()      # Close file

exit1 = open("file1.txt", "r+") # Set read condition for file 1
savedLetters1 = exit1.read(12); # Store file content in a variable
print "\n=> Random letters saved in file 1:", savedLetters1; # Display them
exit1.close() #Close file

##########################################################################
# CREATE AND DISPLAY FILE 2
file2 = open("file2.txt", "wb") # Creates or overwrite file 2
for index in range(0,10):       # For loop with 10 iterations
    # Get a random lowercase letter from the alphabet
    ramdonLetter2 = random.choice(string.ascii_lowercase);
    file2.write(ramdonLetter2); #write random letter on the file
file2.write("\n"); # Make final character a newline character.
file2.close()      # Close file

exit2 = open("file2.txt", "r+") # Set read condition for file 2
savedLetters2 = exit2.read(12); # Store file content in a variable
print "=> Random letters saved in file 2:", savedLetters2; # Display them
exit2.close() #Close file

##########################################################################
# CREATE AND DISPLAY FILE 3
file3 = open("file3.txt", "wb") # Creates or overwrite file 3
for index in range(0,10):       # For loop with 10 iterations
    # Get a random lowercase letter from the alphabet
    ramdonLetter3 = random.choice(string.ascii_lowercase);
    file3.write(ramdonLetter3); #write random letter on the file
file3.write("\n"); # Make final character a newline character.
file3.close()      # Close file

exit3 = open("file3.txt", "r+") # Set read condition for file 3
savedLetters3 = exit3.read(12); # Store file content in a variable
print "=> Random letters saved in file 3:", savedLetters3; # Display them
exit3.close() #Close file

##########################################################################
# PRODUCT CALCULATION
print "\nNOW, LET'S CALCULATE THE PRODUCT OF TWO RANDOM NUMBERS:\n"
randNum1=random.randint(1,42); # Get a random integer from 1 to 42.
print "=> 1st Random Number = %s" % randNum1 # Display it.
randNum2=random.randint(1,42); # Get a random integer from 1 to 42.
print "=> 2nd Random Number = %s" % randNum2  # Display it.
productResult=randNum1*randNum2 # Make the product calculation
print "=> THE PRODUCT IS:    %s" %productResult # Display it.

print "\n***********************************************\n"

##########################################################################

#THE END




