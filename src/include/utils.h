/*******************************************************************************
	Compile-time configuration
*******************************************************************************/

#define CGL_REPEAT_INSTRUCTION 1498
#define MAX_INSTRUCTIONS 500000

//Quick LOG hack that we can make a proper log system out of later
#define LOG printf("[\e[32m%20s:%5d\e[m]\t", __FILE__, __LINE__); printf

template<typename T> T stringTo(const std::string& s) {
  std::istringstream iss(s);
  T x;
  iss >> x;
  return x;
}

template<typename T> std::string toString(const T& x) {
  std::ostringstream oss;
  oss << x;
  return oss.str();
}




/**************************************
	Network Buffer
**************************************/
class BufferedFd
{
	private:
		int fd;
		char buffer[1024*1024];
		int bufferOffset;
		int bufferLength;
	public:
		BufferedFd(int fd_) : fd(fd_), bufferOffset(0), bufferLength(0) {}
		int read(byte *dest, int bytes) {
			int bytesCopied=0;
			do {
				if (bytes>bufferLength-bufferOffset) {
					/* Migrate all the data to the beginning of the buffer */
					memcpy(&buffer[0],&buffer[bufferOffset],bufferLength-bufferOffset);
					bufferLength-=bufferOffset;
					bufferOffset=0;
					int ret=0;

					/* Read in that many bytes */
					ret=::read(fd,&buffer[bufferLength],sizeof(buffer)-bufferLength);

					/* Deal with errors prematurely */
					if (ret == -1)
						return -1;
					if (ret == 0)
						return 0;/* EOF */

					bufferLength+=ret;
				}
				//printf("Buffer Length:%d  Offset:%d Size:%d\n",bufferLength,bufferOffset,sizeof(buffer));

				int todo=bytes;

				if (todo > bufferLength)
					todo = bufferLength;

				memcpy(dest,&buffer[bufferOffset],todo);
				bufferOffset+=todo;
				dest+=todo;
				bytes-=todo;
				bytesCopied+=todo;

			} while (bytes>0);
			//printf("bout to return bytes copied:%d\n",bytesCopied);
			return bytesCopied;
		}

		int write(byte *src, int bytes) {
			int ret;
			ret=::write(fd, src, bytes);
			return ret;
		}
};
