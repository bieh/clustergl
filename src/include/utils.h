/**************************************
		Network Buffer
**************************************/
class BufferedFd {
 private:
#ifdef MULTICAST
   struct sockaddr_in saddr;
#endif
  int fd;
  char buffer[1024*1024*8];
  int bufferOffset;
  int bufferLength;
 public:
#ifdef MULTICAST
 BufferedFd(int fd_,  struct sockaddr_in saddr_) : fd(fd_), saddr(saddr_), bufferOffset(0), bufferLength(0) {}
#else
 BufferedFd(int fd_) : fd(fd_), bufferOffset(0), bufferLength(0) {}
#endif
  int read(byte *dest, int bytes)
  {
	    //printf("called read\n");
    int bytesCopied=0;
    do {
      if (bytes>bufferLength-bufferOffset) {
	/* Migrate all the data to the beginning of the buffer */
	memcpy(&buffer[0],&buffer[bufferOffset],bufferLength-bufferOffset);
	bufferLength-=bufferOffset;
	bufferOffset=0;
	int ret=0;
	   // printf("Recieve(utils)\n");
#ifdef MULTICAST
//   	ret=recvfrom(fd, &buffer[bufferLength], sizeof(buffer)-bufferLength, 0, 
//	     (struct sockaddr *)&saddr, (socklen_t *) sizeof(struct sockaddr_in));

   	ret=::recv(fd, &buffer[bufferLength], sizeof(buffer)-bufferLength, 0);
	    printf(">>Recieve(utils)\n");
#else
	/* Read in that many bytes */
	ret=::read(fd,&buffer[bufferLength],sizeof(buffer)-bufferLength);
#endif
	
	/* Deal with errors prematurely */
	if (ret == -1)
	  return -1;
	if (ret == 0)
	  return 0; /* EOF */
	
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
  
  int write(byte *src, int bytes){
    int ret;
	   // printf("send(utils)\n");
#ifdef MULTICAST
if (bytes > 1024*1024*8-bufferLength){
	    printf("clearbuffer\n");
    ret=::sendto(fd, buffer, bufferLength, 0,
	     (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
memset (buffer, '0', bufferLength);	
bufferLength =0;
}
	
bufferLength += bytes;
      memcpy(&buffer,src,bytes);
	    printf(">>send(utils)\n");
#else
    ret=::write(fd, src, bytes);			
#endif
    return ret;
  }
};
