#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#define BUFSIZE 2048

sigset_t ens;
struct sigaction action;

/*
 *  void parser(char* s,int debut,int fin,char *tab[],int n)
 *  permet de stocker le mot s dans le tableau tab
 */
void parser(char* s,int start_word,int end_word,char *tab[],int n){
    int v = 0;
    for(int i = start_word; i < end_word; i++){
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
	int start_word,end_word;
	int n=0;
    int space_word = 0;
	start_word=0;
	for(int i = 0; i < strlen(s); i++){
		if(s[i]==' '){
			end_word=i;
			parser(s,start_word,end_word,*argv,n);
			start_word=end_word+1;
			n++;
            space_word++;
		}

	}
	parser(s,start_word,strlen(s),*argv,n);
    argv[0][space_word+1] = NULL;
    return 1;
}

/*
 * int redirection(char** tab)
 * permet la redirection d'une commande dans un fichier
 * si le avant dernier mot de tab est ">"
 * et le nom du fichier est le dernier mot de tab
 */
int redirection(char** tab){
    int i = 0;
    while(tab[i] != NULL)
        i++;
    if(strcmp(tab[i-2],">") != 0 )
        return 0;
    int file_descriptor = open(tab[i-1],O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    tab[i-2] = NULL;
        if(file_descriptor == -1)
            return -1;
        dup2(file_descriptor,STDOUT_FILENO);
        execvp(tab[0],tab);
        return file_descriptor;
}

/*
 * void command_chainage(char** words)
 * permet de rediger la permier commande
 * vers la sortie de la deuxieme commande
 */
void command_chainage(char** words){
    int pipes[2];
    pipe(pipes);
    char** buf=malloc(sizeof(char)*BUFSIZE);
    char buffer[BUFSIZE];
    for(int y=0;y<17;y++){
        buf[y]=malloc(sizeof(char)*1024);
    }
    int i = 0;
    int c = 0;
    while(strcmp(words[i],"|") != 0)
        i++;
    c = i;
    i++;
    int y = 0;
    while(words[i] != NULL){
        buf[y] = words[i];
        i++;
        y++;
    }
    buf[y] = NULL;
    words[c] = NULL;
    int processus;
    if( (processus = fork()) == -1)
        return;
    else if(processus == 0){
        dup2(pipes[1],STDOUT_FILENO);
        execvp(words[0],words);
    }else{
        wait(NULL);
        int n = read(pipes[0],buffer,BUFSIZE);
        write(STDIN_FILENO,buffer,n);
        execvp(buf[0],buf);
    }
    free(buf);
}

/*
 * void handler()
 * lors de la reception du signal on sautera d'une ligne
 */
void handler(){
		printf("\n");
}

/*
*int execute_command(char* buffer, char** words)
* permet d'executer des commandes donner par buffer ou words
*/
int execute_command(char* buffer, char** words){
    int processus;
    if( (processus = fork()) == -1)
            return -1;
    else if(processus == 0){
        if(parse_line(buffer,&words) == 0){
            if(execlp(buffer,buffer,NULL) == -1)
                return -1;
        }else{
            if(strpbrk(buffer,"|") != NULL)
                command_chainage(words);
            else{
                if((redirection(words) == 0))
                    if(execvp(words[0],words) == -1)
                        return -1;
            }
        }
    }else
        wait(NULL);
    return 0;
}



int main(int argc, char* argv[]){
    int byte_read,space_word;
    space_word = 0;
    
    sigemptyset(&action.sa_mask);
    action.sa_flags=SA_RESTART;
    action.sa_handler=handler;
    sigaddset(&action.sa_mask,SIGINT);
    sigaction(SIGINT,&action,NULL);
    
    char buffer[BUFSIZE] = "";
    do{
        if( buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i' && buffer[3] == 't')
            exit(1);
        int i = 0;
        //permet de compter le nombre d'espace
        while(buffer[i] != '\0'){
            if(buffer[i] == ' ')
                space_word++;
            i++;
        }
        buffer[i-1] = '\0';
        //int fd;
        char** words = malloc(sizeof(char)*(strlen(buffer) + space_word) );
        for(int y = 0; y < 17; y++){
			words[y] = malloc(sizeof(char)*1024);
		}
        //cree un nouveau processus
        //et agis en fonction de son type(erreur,fils,pere)
        if(execute_command(buffer,words) == -1)
            return -1;
        if(write(STDOUT_FILENO,"$ ",2) == -1)
            return -1;
        free(words);
        //permet de vider le buffer
        for(int i = 0; i < BUFSIZE; i++){
            buffer[i] = 0;
        }
    } while( (byte_read = read(STDIN_FILENO,buffer,BUFSIZE) ) > 0 );
}
