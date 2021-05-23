	printf("Testing fclose...\n");
	fclose(file_w1);
	fclose(file_w2);
	fclose(file_w3);
	for (int i = 3; i <=5; i++) 
	{
		retval = fstat(i, &file_state);
		printf("d = %d, fstat = %d\n", i, retval);
	}