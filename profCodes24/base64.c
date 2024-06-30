#include <stdio.h>


char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char convert(char byte) {
	return table[byte];		
}



void compute_64(unsigned char* in, int len, char* out){
	
	if(len == 3) {
	
		out[0] = convert(in[0] >> 2);
		out[1] = convert(((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4));
		out[2] = convert(((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6));
		out[3] = convert(in[2] & 0x3F);
	
	}
	else if(len == 2) {
	
		out[0] = convert(in[0] >> 2);
		out[1] = convert(((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4));
		out[2] = convert((in[1] & 0x0F) << 2);
		out[3] = '=';


	}
	else if(len == 1) {
		out[0] = convert(in[0] >> 2);
		out[1] = convert((in[0] & 0x03) << 4);
		out[2] = '=';
		out[3] = '=';

	}

}

int main(void) {
	char in[3];
	char out[5];
	out[4] = 0;

	int counter = 0;
	unsigned int c;

	while((c = fgetc(stdin)) != EOF) {
		
		
		in[counter++] = (unsigned char) c;

		if(counter == 3) {
			compute_64(in, 3, out);
			
			printf("%s", out);
			counter = 0;
		}

	}
	
	if(counter != 0){
	       	compute_64(in, counter, out);
		printf("%s\n", out);	
	}
}

