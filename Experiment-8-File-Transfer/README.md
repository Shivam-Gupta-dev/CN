# Socket file transfer

This experiment transfers text, audio, and video files using a TCP
client/server model. Files are opened in binary mode, so the same program
works for all three file types. The client sends a small metadata header
(file name and file size), followed by the file contents in 4096-byte chunks.
Both programs print every chunk's number, size, and cumulative progress.

## Build

Run these commands inside this directory:

```text
gcc -std=c11 -Wall -Wextra -pedantic server.c -o server
gcc -std=c11 -Wall -Wextra -pedantic client.c -o client
```

## Run

Start the server first. The first optional argument is the output directory,
and the second is the port. The server creates the output directory if it does
not already exist:

```text
./server received-files 9003
```

In another terminal, send any text, audio, or video file:

```text
./client 127.0.0.1 notes.txt 9003
./client 127.0.0.1 song.mp3 9003
./client 127.0.0.1 lecture.mp4 9003
```

The received file is first written as `.filename.part`. It is renamed to the
final file name only after all advertised bytes have arrived, so an interrupted
transfer does not appear as a completed file.

## Example chunking output

For a 10,000-byte file:

```text
File: lecture.mp4 (video, 10000 bytes)
Chunk size: 4096 bytes
Chunk 1: sent 4096 bytes (4096/10000 bytes,  40%)
Chunk 2: sent 4096 bytes (8192/10000 bytes,  81%)
Chunk 3: sent 1808 bytes (10000/10000 bytes, 100%)
Transfer complete: 10000 bytes in 3 chunks.
```

The server displays the corresponding receive process:

```text
Receiving lecture.mp4 (10000 bytes) in 4096-byte chunks...
Chunk 1: received 4096 bytes (4096/10000 bytes,  40%)
Chunk 2: received 4096 bytes (8192/10000 bytes,  81%)
Chunk 3: received 1808 bytes (10000/10000 bytes, 100%)
Transfer complete: lecture.mp4 saved (10000 bytes, 3 chunks).
```
