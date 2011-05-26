/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUFSIZE			(16*1024)
#define IMG_SIZE		(16*1024)
#define HEADER_SIZE		16
#define FILE_PERM		(S_IRUSR | S_IWUSR | S_IRGRP \
				| S_IWGRP | S_IROTH | S_IWOTH)

int main(int argc, char **argv)
{
	int i, len;
	unsigned char buffer[BUFSIZE] = {0};
	int ifd, ofd;
	unsigned int checksum = 0, count;

	if (argc != 3) {
		printf(" %d Wrong number of arguments\n", argc);
		exit(EXIT_FAILURE);
	}

	ifd = open(argv[1], O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	ofd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, FILE_PERM);
	if (ifd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			argv[0], argv[2], strerror(errno));
		if (ifd)
			close(ifd);
		exit(EXIT_FAILURE);
	}

	len = lseek(ifd, 0, SEEK_END);
	lseek(ifd, 0, SEEK_SET);

	count = (len < (IMG_SIZE - HEADER_SIZE))
		? len : (IMG_SIZE - HEADER_SIZE);

	if (read(ifd, buffer + HEADER_SIZE, count) != count) {
		fprintf(stderr, "%s: Can't read %s: %s\n",
			argv[0], argv[1], strerror(errno));

		if (ifd)
			close(ifd);
		if (ofd)
			close(ofd);

		exit(EXIT_FAILURE);
	}

	for (i = 0; i < IMG_SIZE - HEADER_SIZE; i++)
		checksum += buffer[i+16];

	*(ulong *)buffer = 0x1f;
	*(ulong *)(buffer+4) = checksum;

	buffer[0] ^= 0x53;
	buffer[1] ^= 0x35;
	buffer[2] ^= 0x50;
	buffer[3] ^= 0x43;
	buffer[4] ^= 0x32;
	buffer[5] ^= 0x31;
	buffer[6] ^= 0x30;
	buffer[7] ^= 0x20;
	buffer[8] ^= 0x48;
	buffer[9] ^= 0x45;
	buffer[10] ^= 0x41;
	buffer[11] ^= 0x44;
	buffer[12] ^= 0x45;
	buffer[13] ^= 0x52;
	buffer[14] ^= 0x20;
	buffer[15] ^= 0x20;

	for (i = 1; i < HEADER_SIZE; i++)
		buffer[i] ^= buffer[i-1];

	if (write(ofd, buffer, BUFSIZE) != BUFSIZE) {
		fprintf(stderr, "%s: Can't write %s: %s\n",
			argv[0], argv[2], strerror(errno));

		if (ifd)
			close(ifd);
		if (ofd)
			close(ofd);

		exit(EXIT_FAILURE);
	}

	if (ifd)
		close(ifd);
	if (ofd)
		close(ofd);

	return EXIT_SUCCESS;
}
