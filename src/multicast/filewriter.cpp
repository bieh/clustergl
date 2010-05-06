#include "main.h"

void textFileWrite(const char *filename, const char *contents) 
	{
		FILE *fp;

		if (filename != NULL) {

			fp = fopen(filename,"a");

			if (fp != NULL) {
										      
				fprintf(fp, contents, filename);
				fclose(fp);
										
			}
		}
	}

unsigned char *textFileRead(const char *filename) 
	{
		FILE *fp;
		unsigned char *content = NULL;

		int count=0;

		if (filename != NULL) {

			fp = fopen(filename,"rt");

			if (fp != NULL) {
										      
        	      		fseek(fp, 0, SEEK_END);
        			count = ftell(fp);
        			rewind(fp);

				if (count > 0) {
					content = (unsigned char *)malloc(sizeof(unsigned char) * (count+1));
					count = fread(content,sizeof(char),count,fp);
					content[count] = '\0';
				}
				fclose(fp);
										
			}
		}
	
		return content;
	}

int main(int argc, char *argv[])
{

	/* set up multicast client */
	Client* c = new Client();

	/* read in the file into memory */
	unsigned char * origFileContents = textFileRead("171.png");
	struct timeval start, end;
	long mtime, seconds, useconds;    

	gettimeofday(&start, NULL);

	/* read the file from multicast */
	unsigned char * fileContents = (unsigned char *) malloc(5108195);
	c->read(fileContents, 5108195);

	/* compare vs. original */
	int cmp = memcmp(origFileContents, fileContents, 5108195);
	printf("contents: %d\n", cmp);
	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;

	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

	printf("Elapsed time: %ld milliseconds\n", mtime);
	


/* close the connection */

return 0;
}
