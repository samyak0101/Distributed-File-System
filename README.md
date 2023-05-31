# Distributed File System Project 🌐

This project is part of the Fall 2022 OS p4 course. We are on the exciting journey of building our own Distributed File System!

## Project Status 🚧

We have finished implementing this project! Clone this repo and compile the files to test it out!

## Helpful Commands 📚

Here are some useful commands for project development:

- To check port numbers and the file descriptor of open ports: `ss -ulpn`
- To read the entire file into memory using `mmap` and flush it to disk with `msync`
- To check the layout of the file system image: `./mkfs -ffs.img -d32 -i32 -v` (you'll need to compile `mkfs` first!)
- To compile the client: `gcc -o out mfs.c udp.c client.c`
- To set the inode to -1 in `unlink`: `in unlink set inode to -1`.

## System Design Overview 🔭

This distributed file system consists of the following components:

1. **Client:** Handles communication with the server.
2. **Server:** Receives messages from the client, sends responses.
3. **Superblock:** Contains metadata about the file system and interactions with the OS.
4. **Inodes:** Data structures that contain metadata about the file system. 
5. **File contents:** Stores file data in blocks, along with associated metadata in Inodes.

For a detailed description of the system architecture, please refer to our [system design document](LINK_TO_DESIGN_DOC).

## Creat Method Notes 📝

We found some edge cases during the implementation of the `creat` method:

- Check if the file or directory already exists before creating new space. 
- Ensure that parent inum exists and is valid.
- Loop through parent inode direntries blocks to find free space for another dir ent.

(For a more detailed walk-through, refer to the full list of `creat` notes in the [project documentation](LINK_TO_PROJECT_DOC).)

## Frequently Asked Questions ❓

- "If data is full, do we still allocate file inode?" 
- "How do we handle mmap sync or flush?" 
- "How do I know for sure if there's a dir entry at a specific location or not?"

For answers to these and more questions, refer to our [FAQs document](LINK_TO_FAQ).

## Contribute 🎁

We welcome all kinds of contributions! Check out the [CONTRIBUTING.md](LINK_TO_CONTRIBUTING) to get started.

## Contact 📬

Having trouble with the project? Got any questions? Feel free to [contact us](mailto:your-email@example.com).

*This project is under the [MIT license]().*
