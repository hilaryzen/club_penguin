# Club Penguin
Welcome to your very own terminal-based club-penguin themed chat room!
## Team Puffles
**Period 4's** home team  
[Kiran](https://github.com/kiran-vuksanaj): Networking extraordinaire  
[Hilary](https://github.com/hilaryzen): Game window goddess  
[Alma](https://github.com/almathaler): Chat window queen  

## Project Description  
The main feature of our project is a networked chat room, allowing the user both text interaction  
and virtual interaction with other players. Our project features a forking server, which handles  
user information packet-style. Our program runs solely in your terminal window -- __please do not attempt to resize it while playing!__

## NCURSES -- How to install and link
To install ncurses on your machine, just enter the following into your terminal:   
```bash
sudo apt-get install libncurses5-dev
```
Other than this, ncurses is linked within our makefile. No other action is required!  

## How to Use  
Welcome! There are a few things you need to know before logging on to Team Puffle's Club Penguin:  
- After installing ncurses, (see above) compile the program using `make`
- Launch a server by running `./server`; on some machines, this may require firewall permission
- Join said server using `./client <IPv4>`, placing the IP address of the server's computer in place of `<IPv4>`. If this argument is left blank, it will default to server searching on localhost.

Once you've logged on, you can:
- Move your cursor around with the F1-4 keys (on some machines, hold Fn+F1-4 to use function keys)
- Type your chat message in the typing window, and hit enter when done!
- Move your avatar around with the arrow keys
- Socialize, and enjoy your time here as a colorful aquatic flightless bird

## Known Bugs/Usage Restrcitions
Within the chat:  
1. You may use up, down, right and left arrow keys to modify your cursor position. However, if you   
   have multiple lines of text in your message, you must use up and down keys to switch the vertical  
   position of your cursor.

2. If you have multiple lines in your message, you may use backspace or the delete key to delete a whole line
   and move in to the previous. However, if you first modify the cursor position horizontally, backspace will
   still move in to the previous line, but the cursor will not reflect this.

3. You can move your cursor into the chat window, but you cannot scroll through it.
