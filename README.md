# club_penguin
# Names: Kiran Vuksanaj, Hilary Zen and Alma Thaler
# Group Name: Puffles

Our project seeks to revive the recently deceased multiplayer game/chatroom, Club Penguin.

All interaction will occur through the user’s terminal. On the left-hand side will be an ‘igloo’, populated by the user’s avatar and other users’ avatars too. On the right-hand side will be a scrollable chat which each user can interact with. Our bonus end goal is having mini-games, but the main feature of our project is a networked chat room and avatar movement that appears seamless.

In this project, we plan to use networking, semaphores, pipes, signals, and multiple processes. Using networking will enable multiple players to join the same game from different computers and interact with each other, while semaphores will be used to limit the number of users connected at a time. On the server side, one subprocess will be dedicated to each user, listening for messages received from the user and, upon reception, interpreting the message, updating the board in shared memory (write access to be semaphore controlled), and writing a new message to an “outbox queue” (whose write access will also be controlled by a semaphore). Another process will be dedicated to processing the outbox, writing the message to each connected socket.

We also plan to use structs for each avatar that will contain information like its position on the screen, and for different types of messages to be sent through sockets, such as a change of position or a chat message. The board that each player sees will continuously update based on these structs.

We have divided the project into three main parts. Kiran will implement the networking aspect, Alma will build the chat feature, and Hilary will create the terminal display of avatars. Since the chat and avatar display are all on one screen, Hilary and Alma will also need to work together to integrate those two aspects. One challenge may be implementing a scrolling chat, while the igloo display remains static in position.

Timeline
Jan 6: Player can start the program and move an avatar on the screen
Jan 8: Add a chat where the player can type messages and see them on the screen
Jan 13: Complete networking so that multiple players can join the game
Jan 15: All players can type into the chat, but only one message is added at a time
Jan 17: Add different rooms where players can go into
(Extra) Add some minigames that only involve one player
(Extra) Scrollable chat
