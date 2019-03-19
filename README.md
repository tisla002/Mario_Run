# CS120B Final Project

## Introduction
Mario Run is a side-scrolling game inspired by Super Mario Bros. The objective of the game is to move the player to the end of the level by crossing obstacles and barriers. The player controls the character using a bluetooth controller and jumps across obstacles to reach the end of the level in the fastest time possible while trying not to die. The game itself is displayed on LED Matrix, while information pertaining to the game (score etc.) is displayed on a LCD screen. There is Mario music playing while the game is running, and the bluetooth controller includes jump, move, start, and reset buttons.

<img src="https://github.com/tisla002/cs120B_final_project/blob/master/Images/Game%20Start.jpg" width="200" height="250"> <img src="https://github.com/tisla002/cs120B_final_project/blob/master/Images/Game%20Running.jpg" width="200" height="250">

## Hardware
### Parts List
The hardware that was used in this design is listed below. 

* ATMega1284 microcontroller
* 4-Pin LCD Display
* LED Matrix
* Speaker
* Bluetooth Module


## Pinout
<img src="https://github.com/tisla002/cs120B_final_project/blob/master/Images/Pinout%20Diagram.jpg" width="250" height="300">

## Software
State Machine functionality left out of images, all images are within images folder.

<img src="https://github.com/tisla002/cs120B_final_project/blob/master/Images/Display%20SM.JPG" width="300" height="200"> <img src="https://github.com/tisla002/cs120B_final_project/blob/master/Images/Output%20SM.JPG" width="150" height="150">

<img src="https://github.com/tisla002/cs120B_final_project/blob/master/Images/Bluetooth%20SM.JPG" width="200" height="200"> <img src="https://github.com/tisla002/cs120B_final_project/blob/master/Images/Song%20SM.JPG" width="200" height="200">

## Complexities

### Completed Complexities:
* Led Matrix
* Bluetooth Module (USART)
* Using EEPROM to save the high score (minimum time)
* LCD 4-Pin DATA
* Core Game Logic

### Incomplete complexities:
* Nokia 5110 (wouldn’t arrive on time, changed to LED Matrix)

## Video
CS 120B Project Youtube [Link](https://youtu.be/u_sordWC2No)

## Known Bugs and Shortcomings
* After you reset the game, sometimes only one LED in the Matrix is still lit up. I believe that this is caused be me not resetting the Row and Column outputs properly.
* After you first program the game with your programmer, there is garbage values shown on the LCD screen. Resetting the power fixes this problem. The issue could be due to the timing of each tasks.

## Future work
In future versions of this project, I would add multiple levels, enemies that I can destroy, as well as more music. I would also like to add more intuitive controls, and switch from LED Matrix to a Nokia 5110.

