#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#define BUFSIZE 2048

/*
 *  void parser(char* s,int debut,int fin,char *tab[],int n)
 *  permet de stocker le mot s dans le tableau tab
 */
void parser(char* s,int debut,int fin,char *tab[],int n){
    int v=0;
    for(int i=debut;i<fin;i++){
        tab[n][v]=s[i];
        v++;
    }
    tab[n][v] = '\0';
}

/*
 *  int parse_line(char*s, char** argv[])
 *  permet de séparer la chaine s s'il y a des espaces
 *  puis de stocker les mots dans le tableau argv[]
 */
int parse_line(char*s,char** argv []){
	if(strpbrk(s," ")==NULL) return 0;
	int debut,fin;
	int n=0;
    int space = 0;
	debut=0;
	for(int i=0;i<strlen(s);i++){
		if(s[i]==' '){
			fin=i;
			parser(s,debut,fin,*argv,n);
			debut=fin+1;
			n++;
            space++;
		}

	}
	parser(s,debut,strlen(s),*argv,n);
    argv[n]=NULL;
    argv[0][space+1] = NULL;
    return 1;
}

int create_file(char** argv,char* buffer){
    int fd = 0;
    int i = 0;
    char** arg;
    int copy;
    while(argv[i] != NULL){
        arg[i] = argv[i];
        i++;
    }
    if(strcmp(argv[i-2], ">") == 0){
        fd = open(argv[i-1],O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        copy = dup(STDOUT_FILENO);
        close(STDOUT_FILENO);
        arg[i-2] = NULL;
        arg[i-1] = NULL;
        execvp(arg[0],arg);
        int byte_read_redirection = read(copy,buffer,sizeof(buffer));
        if(byte_read_redirection == -1)
            return -1;
        if( write(fd,buffer,byte_read_redirection) == -1)
            return -1;
        close(fd);
        dup2(copy,STDOUT_FILENO);
        close(copy);
        return 0;
    }
    return 1;
}


int main(int argc, char* argv[]){
    int byte_read,processus,space;
    space = 0;
    char buffer[BUFSIZE] = "";
    do{
        if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i' && buffer[3] == 't')
            exit(1);
        int i = 0;
        //permet de compter le nombre d'espace
        while(buffer[i] != '\0'){
            if(buffer[i] == ' ')
                space++;
            i++;
        }
        buffer[i-1] = '\0';
        char** tab=malloc(sizeof(char)*(strlen(buffer) + space) );
        for(int i=0;i<17;i++){
			tab[i]=malloc(sizeof(char)*1024);
		}
        int parse = parse_line(buffer,&tab);
        //cree un nouveau processus
        //et agis en fonction de son type(erreur,fils,pere)
        if( (processus = fork()) == -1)
            return -1;
        else if(processus == 0){
            if(parse == 0){
                if(execlp(buffer,buffer,NULL) == -1)
                    return -1;
            }else{
                if(execvp(tab[0],tab) == -1)
                    return -1;
            }
        }else{
            wait(NULL);
            if(write(STDOUT_FILENO,"$ ",2) == -1)
                return -1;
        }
        free(tab);
        //permet de vider le buffer
        for(int i = 0; i < BUFSIZE; i++){
            buffer[i] = 0;
        }
    } while( (byte_read = read(STDIN_FILENO,buffer,BUFSIZE) ) > 0 );
}

/*
 * if(strcmp(tab[avant dernier],"<") == 0){
 *      int fd = open("ficher dernier",O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
 *      if(fd == -1)
 *          return -1;
 *      int copy = dup(STDOUT_FILENO);
 *      close(STDOUT_FILENO);
 *      int byte_read_redirection = read(copy,buffer,BUFSIZE);
 *      write(fd,buffer,byte_read_redirection);
 *      close(fd);
 *      dup2(copy,STDOUT_FILENO);
 *      close(copy);
 * }
 *
 *
 *  creer fichier
 *  copier STDOUT_FILENO dans ficher creer
 *  fermer STDOUT_FILENO
 *  lire et ecrire dans fichier
 *  ouvrir STDOUT_FILENO
 *  fermer fichier
 *
 */
