/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include "sodium.h"

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void printArr(unsigned char* arr, int len)
{
	for(int i = 0; i<len; i++)
	{
		printf("%02x", arr[i] & 0xff);
		if(i % 4 == 3) printf(" ");
	}
}

int inflateFFs(const unsigned char* buf, int size, unsigned char* out)
{
	int writeHead = 0;
	bool prevCharIs0d = false;
	for(int i = 0; i<size; i++)
	{
		//if(buf[i] == 0xff && (!prevCharIs0d))
		//{
		//	out[writeHead] = 0xff;
		//	out[writeHead + 1] = 0xff;
		//	writeHead += 2;
		//}
		//else if(buf[i] == 0x00 && prevCharIs0d)
		//{
		//	out[writeHead] = 0x00;
		//	out[writeHead + 1] = 0x00;
		//	writeHead += 2;
		//}
		//else
		{
			out[writeHead] = buf[i];
			writeHead++;
		}
		
		if(buf[i] == 0x0d)
		{
			prevCharIs0d = !prevCharIs0d;
		}
		else
		{
			prevCharIs0d = false;
		}
	}
	return writeHead;
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
   	
	char key[32];
	{
		FILE* keyFile = fopen(CHANGE ME TO KEY LOCATION, "r");
		if(!keyFile)
		{
			printf("Couldn't open Key!\n");
			abort();
		}
		for(int i = 0; i<32; i++)
		{
			key[i] = getc(keyFile);
		}
		fclose(keyFile);
	}
    printf("Key read!\n");

    while(1) {  // main accept() loop
		if (sodium_init() == -1) {
			return 1;
		}
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
		
        printf("A");fflush(stdout);
		FILE *tasks = fopen("./Tasks.txt", "r");
		if(!tasks)
		{
			printf("Couldn't open Tasks!\n");
			abort();
		}
        printf("B");fflush(stdout);
		fseek(tasks, 0, SEEK_END);
        printf("C");fflush(stdout);
		const long MESSAGE_LEN = ftell(tasks);
        printf("D");fflush(stdout);
		fseek(tasks, 0, SEEK_SET);
        printf("E");fflush(stdout);
		
        printf("Message size: %d", MESSAGE_LEN);fflush(stdout);
		
		char message[MESSAGE_LEN + 1];
		fread(message, MESSAGE_LEN, 1, tasks);
		fclose(tasks);
        printf("Message read.");
		
		message[MESSAGE_LEN] = 0;
		
		unsigned char sendBuffer[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + MESSAGE_LEN + crypto_aead_xchacha20poly1305_ietf_ABYTES];
		unsigned char* nonce = sendBuffer;
		randombytes_buf(nonce, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
		//for(int i = 0; i<crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; i++)
		//{
		//	if(nonce[i] < 90) nonce[i] = 0x00;
		//	else if(nonce[i] < 180) nonce[i] = 0xff;
		//	else nonce[i] = 0x0d;
		//}
		//nonce[0] = 0x0d;
		//nonce[1] = 0x0d;
		//nonce[2] = 0x00;
		
		//int ix = 0;
		//nonce[ix++] = 0x0d;
		//nonce[ix++] = 0x00;
		//nonce[ix++] = 0xff;
		//nonce[ix++] = 0x96;
		//nonce[ix++] = 0x95;
		//nonce[ix++] = 0xae;
		//nonce[ix++] = 0x9c;
		//nonce[ix++] = 0x34;
		//nonce[ix++] = 0x88;
		//nonce[ix++] = 0xb8;
		//nonce[ix++] = 0x39;
		//nonce[ix++] = 0xa9;
		//nonce[ix++] = 0xf0;
		//nonce[ix++] = 0x88;
		//nonce[ix++] = 0x30;
		//nonce[ix++] = 0xed;
		//nonce[ix++] = 0x88;
		//nonce[ix++] = 0x9c;
		//nonce[ix++] = 0x9f;
		//nonce[ix++] = 0xa3;
		//nonce[ix++] = 0xbe;
		//nonce[ix++] = 0xfd;
		//nonce[ix++] = 0x89;
		//nonce[ix++] = 0xce;
		
        printf("Nonce created!\n", s);
		
		unsigned char* ciphertext = sendBuffer + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
		unsigned long long ciphertext_len;
	
		crypto_aead_xchacha20poly1305_ietf_encrypt(ciphertext, &ciphertext_len,
			(const unsigned char*)message, MESSAGE_LEN,
			nullptr, 0,
			NULL, nonce, (const unsigned char*)key);
        printf("Encryption created!\n");
		printf("Cipher: ");
		printArr(ciphertext, ciphertext_len);
		printf("\n");
		printf("Nonce: ");
		printArr(nonce,crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
		printf("\n");
		
		
		unsigned char inflatedSendBuffer[sizeof(sendBuffer) * 2];
		
		int inflatedSendBufferSize = inflateFFs(sendBuffer, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + ciphertext_len, inflatedSendBuffer);
		
		printf("Inflated: ");
		printArr(inflatedSendBuffer,inflatedSendBufferSize);
		printf("\n");
		
		if (send(new_fd, inflatedSendBuffer, inflatedSendBufferSize, 0) == -1)
			perror("send");
		
        printf("Encryption sent!\n");
		
		shutdown(new_fd, SHUT_WR);
        printf("Shutdown!\n");
        printf("Read!\n");
        close(new_fd);
        printf("Close!\n\n");
    }

    return 0;
}
