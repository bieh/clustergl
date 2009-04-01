/**************************************
		Network Buffer
**************************************/
class BufferedFd {
	private:
		int fd;
		char buffer[1024*1024*8];
		int bufferOffset;
		int bufferLength;
	public:
		BufferedFd(int fd_) : fd(fd_), bufferOffset(0), bufferLength(0) {}
		int read(byte *dest, int bytes)
		{
			int bytesCopied=0;
			do {
				if (bytes>bufferLength-bufferOffset) {
					/* Migrate all the data to the beginning of the buffer */
					memcpy(&buffer[0],&buffer[bufferOffset],bufferLength-bufferOffset);
					bufferLength-=bufferOffset;
					bufferOffset=0;
					
					/* Read in that many bytes */
					int ret=::read(fd,&buffer[bufferLength],sizeof(buffer)-bufferLength);
					
					/* Deal with errors prematurely */
					if (ret == -1)
						return -1;
					if (ret == 0)
						return 0; /* EOF */
						
					bufferLength+=ret;
				}
				
				int todo=bytes;
				
				if (todo > bufferLength)
					todo = bufferLength;
			
				memcpy(dest,&buffer[bufferOffset],todo);
				bufferOffset+=todo;
				dest+=todo;
				bytes-=todo;
				bytesCopied+=todo;
				
			} while (bytes>0);
			
			return bytesCopied;
		}
		
		int write(byte *src, int bytes){
			int ret=::write(fd, src, bytes);			
			return ret;
		}
};
