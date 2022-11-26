#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

// Huzaifa Hashim Karbalai - 18591

void flagL(char* directory) {
DIR *dp;
struct passwd *pws; 
char *c;
struct group *gp;
struct dirent *ep;
struct stat f;
dp = opendir(directory);
stat(directory, &f);
while ((ep=readdir(dp))!=NULL){
stat(ep->d_name,&f);
printf((S_ISDIR(f.st_mode))?"d":"-");
printf((f.st_mode & S_IRUSR) ? "r":"-");
printf((f.st_mode & S_IWUSR) ? "w":"-");
printf((f.st_mode & S_IXUSR) ? "x":"-");
printf((f.st_mode & S_IRGRP) ? "r":"-");
printf((f.st_mode & S_IWGRP) ? "x":"-");
printf((f.st_mode & S_IXGRP) ? "x":"-");
printf((f.st_mode & S_IROTH) ? "r":"-");
printf((f.st_mode & S_IWOTH) ? "w":"-");
printf((f.st_mode & S_IXOTH) ? "x":"-");
printf(" ");
printf("%d ",f.st_nlink);
pws = getpwuid(f.st_uid);
printf("%s ", pws->pw_name);
gp=getgrgid(f.st_gid);
printf("%s ", gp->gr_name);
printf("%d ", f.st_size);

printf(ctime(&f.st_mtime));
printf(" %s\n", ep->d_name);


}
closedir(dp);

}

void flagA(char* directory){ 
char name[100];
DIR *dp;
struct dirent *ep;
dp = opendir(directory);
if (dp !=NULL){
	while(ep = readdir(dp)) {
		// prints hidden files too.
		char type[10];
		if (ep->d_type == DT_DIR)
		strcpy(type,"Dir");
		else
		strcpy(type,"File");
	printf("%s:%s\n",type,ep->d_name);
	}
closedir(dp);
}
else {
puts("This directory cannot be opened.");
}

}

void flagR(const char *name, int indent){
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))
        return;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s[%s]\n", indent, "", entry->d_name);
            flagR(path, indent + 2);
        } else {
            printf("%*s- %s\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
}

void lsOnly(char* directory){
char name[100];
DIR *dp;
struct dirent *ep;
dp = opendir(directory);
if (dp !=NULL){
	while(ep = readdir(dp)) {
		strcpy(name,ep->d_name);
		if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0) || (name[0] =='.')) {
		
		}
else{
		char type[10];
		if (ep->d_type == DT_DIR)
		strcpy(type,"Dir");
		else
		strcpy(type,"File");
	printf("%s:%s\n",type,ep->d_name);
}
	}
closedir(dp);
}
else {
puts("This Directory cannot be opened.");
}

}

int main(void){
char* ls = "ls";
char* l = "l";
char* a = "a";
char* R = "R";
bool isFlagA = false;
bool isFlagL = false;
bool isFlagR = false;
char dir[30]; 
printf("Please enter any of these commands:\nls <dir> \nls -l <dir> \nls -a <dir> \nls -R <dir>\n");
char inputCommand[50];

fgets(inputCommand, sizeof(inputCommand), stdin); 
inputCommand[strcspn(inputCommand, "\n")] = 0;

char* piece = strtok(inputCommand, " -");
while(piece != NULL) {
    if(strcmp(piece, l) == 0){
       isFlagL = true;}
   else if (strcmp(piece, a) == 0){
	isFlagA = true;}
   else if (strcmp(piece, R) == 0){
	isFlagR = true;}
   else if (strcmp(piece, ls) == 0){
	printf("ls command is executed.\n");}
   else {
   strcpy(dir, piece);
   }
    
piece = strtok(NULL, " -");
}
if (isFlagL){
flagL(dir);
}
else if (isFlagA){
flagA(dir);}
else if (isFlagR){
flagR(dir, 0);}
else {
lsOnly(dir);}
return 0;}

// Huzaifa Hashim Karbalai - 18591