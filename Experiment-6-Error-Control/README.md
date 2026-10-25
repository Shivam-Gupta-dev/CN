# Hamming code and CRC error control

These examples use TCP client/server sockets on loopback:

| Program | Server port | Build |
|---|---:|---|
| Hamming code | 9001 | `gcc server.c -o server` and `gcc client.c -o client` |
| CRC | 9002 | `gcc server.c -o server` and `gcc client.c -o client` |

Run the server first, then the client. The client prints the generated frame
and lets you flip one bit before transmission. The Hamming server calculates
the syndrome and corrects one-bit errors. The CRC server divides the received
frame by the supplied generator polynomial and reports whether the remainder
is zero.

On Linux/macOS:

```text
cd Experiment-Error-Control/Hamming
./server                 # terminal 1
./client                 # terminal 2

cd ../CRC
./server                 # terminal 1
./client                 # terminal 2
```

## Hamming code example

Start the server in the first terminal:

```text
$ cd Experiment-Error-Control/Hamming
$ ./server
Hamming server listening on port 9001...
```

Run the client in a second terminal. In this example, data `1011` is encoded
and bit position `3` is flipped before transmission:

```text
$ ./client
Hamming Code Client
Enter data bits: 1011
Generated codeword: 0110011
Enter bit position to flip (1-7, 0 for none): 3
Transmitted codeword: 0100011
Server: Error at bit position 3 corrected. Codeword: 0110011
```

Use `0` as the error position to send the codeword without changing it:

```text
Enter data bits: 1011
Generated codeword: 0110011
Enter bit position to flip (1-7, 0 for none): 0
Transmitted codeword: 0110011
Server: No error detected. Codeword: 0110011
```

## CRC example

Start the CRC server:

```text
$ cd Experiment-Error-Control/CRC
$ ./server
CRC server listening on port 9002...
```

The following example uses data `1101011011` and generator polynomial
`10011` without injecting an error:

```text
$ ./client
CRC Client
Enter data bits: 1101011011
Enter generator polynomial: 10011
Transmitted codeword: 11010110111110
Enter bit position to flip (1-14, 0 for none): 0
Sent frame: 11010110111110
Server: No error detected. Remainder is zero.
```

To demonstrate CRC error detection, repeat the example and flip bit position
`1`:

```text
Enter data bits: 1101011011
Enter generator polynomial: 10011
Transmitted codeword: 11010110111110
Enter bit position to flip (1-14, 0 for none): 1
Sent frame: 01010110111110
Server: Error detected. Remainder: 1101
```

The client also accepts polynomial notation. For example:

```text
Enter data bits: 11001
Enter generator polynomial (for example x^4+x^3+1): x^4+x^3+1
Generator polynomial: 11001
```

The server prints each modulo-2 subtraction step and the final remainder.
