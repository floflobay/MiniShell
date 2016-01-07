//
//  main.c
//  MiniShell
//
//  Created by AntoineAuer on 12/11/2015.
//  Copyright © 2015 AntoineAuer. All rights reserved.
//

//-------------------------//
/* Pb constaté a l execution :
1) Lorsque on ne marque rien il refait l ancienne commande // solve suis au la RAZ de Resp
 

*/


#define SIZEMAX 100
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// entete de fonctions
int parsing(char **resP);
int commande(int fin, int fout, char* resP[], char* param, int* bg);
int delimiteur(int c);
int nli =0 ; //afficher en cas de


// variables globales
// on changera rescommande pour la mettre dans le main
int rescommande = 0;
char* nomFichier[SIZEMAX]; //dans la theorie ici size max vaut 1 car un seul nom de fichier
int redir= -1;//pour la redirection
int redire=-1;

int main(int argc, const char * argv[]) {
    char* resP[SIZEMAX];
    int fin = 0;    
    int fout[1];    fout[0]=1;
    int bg[1];
    // Création d'un fichier récuperant les impressions intermediaires (deboggage)
    int fmess = open("impErreur.txt",O_CREAT | O_RDWR | O_TRUNC, 0666);
    close(2);
    dup(fmess);
    close(fmess);
    char com[20];
    char param[20]; //VAR param ajoutée
    int status, eof = 0;
    
    printf("DAUPHINE> ");
    fflush(stdout);

    while (1) {
        //printf("rescommande = %d", rescommande);fflush(stdout);
        //sortie du prgm
        if(fin == 64){break;}
        //redirection
        
        
        if (rescommande == 2) {
            rescommande=0;
            nli=0;
            printf("DAUPHINE> ");
            fflush(stdout);
        }else{
            nli=0;
            
            fin = commande(0,1,resP,param,bg);
            //on a subi un changement de sortie
            if(!(redir == -1 ) && resP[0]== '\0'){
                close(1);
                dup(redir);
                close(redir);
                redir = -1;
            }
            //on a subi un changement d entree
           
        }
        
       
    }
     return 0;
  }

// pour plus tard mettre des pointeurs sur fin fout, le retour sera rescommande
int commande(int fin, int fout, char* resP[], char* param, int* bg){
    int s,i, res=0, status, eof; // VAR status && eof ajoutées
    int c;
    s=parsing(resP);
    fprintf(stderr, "résultat parsing %d\n",s);
    
    switch (s) {
 
        case 0: // NL`nouvelle lign
            nli = 1;
            pid_t pid2 ;
            if ( (pid2 = fork())==0 ){
                rescommande = 1; //cela permettra de ne pas de reecrire
                execvp(resP[0], resP);
                break;
            }else{
                wait(&status);
                rescommande = 2 ;
                eof = 2;
                resP[0]='\0';// on peut donc nettoyer apres l execution resP .
                break;
            }
            break;
            
        case 1: // ;
            getchar();
            nli+=2;
          // if( nl == 3 ){getchar();}//on prend le ;
             res = 2;
            pid_t pid ;
            if ( (pid = fork())==0 ){
                //printf("\nexecute");fflush(stdout);
                rescommande = 1; //cela permettra de ne pas de reecrire
                execvp(resP[0], resP);
               // printf("\nexecute");fflush(stdout);

            }else{
                wait(&status);
                if(nli==3){
                rescommande = 2 ;
                }else{
                 rescommande = 0;
                }
                resP[0]='\0';// on peut donc nettoyer apres l execution resP .
                eof = 2;
               break;
            }
            
            
            
            break;
            
        case 2: // &
             //TO-DO
            *bg=1;
            break;
            
        case 3: // <
            c = getchar();// enlever le >
            parsing(nomFichier);
            //printf("\nCas 4 : %s %c\n",nomFichier[0],c);fflush(stdout);
            redire = dup(0); //on ne perd pas l entre courante
            close(0);
            i=open(nomFichier[0],O_RDONLY, 0666);//le nom du fichier
            fprintf(stderr,"fichier ouvert : %s canal num : %d\n",nomFichier[0],i);
            dup(i);//comme ca on prendra les infos dessus avec le truc inferieur
            break;
            
        case 4: // >
            //on sait ce qu on doit mettre on ne sait pas ou il faut donc faire un parsing et apres un exec
            c = getchar();// enlever le >
            parsing(nomFichier);
            //printf("\nCas 4 : %s %c\n",nomFichier[0],c);fflush(stdout);
            redir = dup(1); //on ne perd pas la sortie courante
            close(1);
            i=open(nomFichier[0],O_CREAT | O_RDWR | O_TRUNC, 0666);//le nom du fichier
            fprintf(stderr,"fichier ouvert : %s canal num : %d\n",nomFichier[0],i);
            dup(i);//comme ca on ecrira dessus avec le truc superieur
            
            break;
            
        case 5: // |
             //TO-DO
            break;
            
        case 7: // EOF
            
             //TO-DO
            //on arrive a la fin du fichier lu en cas de redire '<'
            if(redire !=-1){
                close(0);
                dup(redire);
                close(redire);
                redire = -1;
                break;
            }else{
                
              //  rescommande =2;// a enlever

            }
            
            
               //               //  return 0;
            break;
            
        case 10: // mot
            if (strcmp(resP[0], "exit")==0) {
               // *fin=1;
                printf("\nEXIT\n");
                return 64;
            }
            break;
            
        default:
            fprintf(stderr, "erreur au switch de commande\n");
            break;
    }
    return res;
}

int parsing(char ** resP){
    char mot[100];
    int i=0;
    int cmot = 0;
    fprintf(stderr, "debut parsing\n");
    
    while (1) {
        int c = getchar();
        fprintf(stderr,"caractere lu %d %c\n",c,c);
        
        if (c == '\n') {
            return 0;
        }
        else if (c == ';') {
            return 1;
        }
        else if (c == '&') {
            return 2;
        }
        else if (c == '<') {
            return 3;
        }
        else if (c == '>') {
            return 4;
        }
        else if (c == '|') {
            return 5;
        }
        else if (c == EOF) {
            return 7;
        }

        else if (c != ' ') {
            // On rentre dans une sequence
            while( c != '\n' && !delimiteur(c) && !(c == EOF) ){
                i=0;
                while (c!=32) {
                    if ((c != '\n') && (delimiteur(c) == 0)) {
                        mot[i] = c;
                        i++;
                        c=getchar();
                        fprintf(stderr, "valeur du caractere lu %d %c\n",c,c);
                        if (c == EOF) {
                            break;
                        }
                    }
                    else{
                        //fprintf(stderr, "break\n");
                        break;
                    }
                }
                    break;
            }
                        
            while(c == ' ')
                c = getchar();
            
            ungetc(c, stdin);
            mot[i] = '\0';
            resP[cmot++] = strdup(mot);
            fprintf(stderr, "element comm lue %s %s\n", resP[0],resP[1]);
            if(c == '\n' || delimiteur(c) ){
                resP[cmot]=0;
                if(c!='\n')
                    ungetc(c, stdin);
                return 10;
            }
        }
    }
    //erreur a traiter
    fprintf(stderr, "element comm lue %s %s\n", resP[0],resP[1]);
    return 404;
}
                        
int delimiteur(int c){
    if(c == '&' || c == ';' || c == '<' || c == '>' || c == '|') // Rearangement plus propre
        return 1;
    return 0;
}
