#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <errno.h>


#define SET_COLOR_RED     "\x1b[31m"
#define SET_COLOR_GREEN   "\x1b[32m" 
#define SET_COLOR_YELLOW  "\033[33m"
#define SET_COLOR_RESET   "\x1b[0m"

int main (void)
{
	const char* mntPoint  = "/mnt/usb-backup";
	const char* base      = "/dev/"; 

	char  partitionLocation[20]; // currently unknown 
	
	printf("\n");
	if   (  system("sudo lsblk -f") != 0 )
	{	
		perror (SET_COLOR_YELLOW "\nlsblk -f\n" SET_COLOR_RESET); 
	} 
 
	printf ("\nPartition to mount (e.g., sdb1): "   );

	char partitionName[10]; 
	fgets  (partitionName, sizeof( partitionName ), stdin);
		
	for (int i = 0; partitionName[i] != '\0'; i++) 
	{
		if (  partitionName[i] == '\n'  )
		partitionName[i] = '\0';  	
	}
	snprintf ( partitionLocation, sizeof (partitionLocation), "%s%s", base, partitionName ); 

	int fd = open ( partitionLocation, O_RDONLY ); 
	if ( fd < 0 )
	{ 
		perror ( SET_COLOR_RED "OPEN FAILED" SET_COLOR_RESET );
		return -1; 
	}
	
	unsigned long long size; 
	if      (  (ioctl(fd, BLKGETSIZE64, &size))    <      0    ) 
	{
		perror(SET_COLOR_RED "IOCTL FAILED" SET_COLOR_RESET); 
		close(fd); 
		return -1; 
	}  
	
	printf ("\n[  PARTITION INFORMATION  ]\n"                           ); 
	printf ("- Name     :  %s\n"             , partitionName            ); 
	printf ("- Location :  %s\n"             , partitionLocation        ); 	
	printf ("- Size     :  %li (%.2fgb)\n\n" , size, size/1073741824.0  );               		
	
	printf (SET_COLOR_YELLOW "WARNING - CHOOSING THE WRONG PARTITION MAY CAUSE PROBLEMS" SET_COLOR_RESET); 
	printf ("\n%s will be mounted on %s", partitionName, mntPoint				            ); 
	printf ("\nAre you sure you want to continue (y/n)? "						    ); 
	
	char userAnswer = getchar(); 
	if      ( userAnswer != 'y' && userAnswer != 'Y' ) 
	{
		printf ("... Operation aborted\n"); 
		close  (fd);
		return 0;
	} 
	
	//      make directory	
	if      ( mkdir (mntPoint, 0755) == -1   &&   errno != EEXIST         )
	{
		perror  ("mkdir failed");
		close   (fd);
		return  -1;
	}

	//      mount partition to directory
	if      ( mount (partitionLocation, mntPoint, "vfat", 0, NULL) == -1  )
	{	
		perror  ("mount failed");
		close   (fd); 
		return  -1;
	}
 	
	
	close(fd); 
	return 0;
}

