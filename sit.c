/** 
*  	 AUTHOR       - Ravi Kumar L
*  	 email        - suryan1607@gmail.com
*  	 Program Name - Snap-Edit (Sit)
*  	 A simple Line-Editor
*/


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define INITIAL_CHAR_MAX 100
#define COMMAND_ERROR 1
#define DELETE 2
#define APPEND 3
#define REPLACE 4
#define QUIT 5
#define WRITE 6
#define PRINT 7
#define LINE_ERROR 8
#define HELP 9

#define LINES_AT_START 30 
#define LINE_NOT_BUILT -1

// Add functionality to offer to save current buffer before exit -- Done

#define HANDLE_CALLOC(p,x,y) 				\
	if (!(p=calloc(x,y)))  			\
	{ 						\
		printf("\n\nInsufficient Memory!"); 	\
		exit(EXIT_FAILURE); 			\
	}

#define HANDLE_REALLOC(p,x) \
				\
	if (!(p=realloc(p,x))) \
	{	\
		printf("\n\nInsufficeint Memory"); \
		exit(EXIT_FAILURE); 	\
	}





typedef struct line {
	int maxsize;                 // Stores max number of characters the line can hold at the current time
	int currentsize;	     // Stores the number of characters in the line
	int thisline;		     // Stores the line number
	char *linestring;	     // pointer to the string which holds the text on the line (string does\ not end with '\n')
	void *next;		     // pointer to the next line struct
} line;

int command=0; //strores the command(integer-mapped) after processing the raw input command
int linenum=0; // stores the line number the command is referring to
line *lastline; //stores a pointer to the last line(structure) in the buffer
line *buffer; //stores the pointer to the first line in the buffer
char filename[32]; //stores the current filename
int savebeforequit; // 1 if changes have been made to the buffer since the last write


//Function declarations
void displayhelp(); 
int write(FILE **); //Writes buffer onto file
int print(); // Prints contents of the buffer onto the concole 
int append(int, char *); // Appends the array at the char * to the passed line
int replace(int , char *); //replaces the content of the passed line number with the passed string 
int delete(int); //free()'s the structure and string of the line number passed and changes the line numbers on the following line structures
void expandstring(line *); //expands the string the line to which pointer is passed
void allocstring(line *); // Allocates string on the pointer to line passed
void buildbuffertill(int);// builds the line structure till the line number passed
line * findline(int); // returns a pointer to the line structure of the line number passed
void processraw(char *,char *); //processes the raw input command string received and identifies command and line number and copies input string if  any into the second pointer to string passed
void displayhelp();
void readfileintobuffer(FILE *);


void readfileintobuffer(FILE *fptr)  
{
	int tracklinenum=1;
	char *tempstr; //stores strings from the file line-by-line
	int tempstrcur=0;
	int tempstrmax=49;
	unsigned char c;
	HANDLE_CALLOC(tempstr,50,sizeof(char));
	
	while(!(feof(fptr))) //checking for end of file
	{
		c=fgetc(fptr); // retreives a character and advances pointer
		if(c=='\n')  //when a whole line hass been copied into the string
		{	replace(tracklinenum,tempstr); 
			memset(tempstr,'\0',tempstrmax);
			tracklinenum++;
			tempstrcur=0;
		}
		else if(tempstrcur<tempstrmax)
		{
			tempstr[tempstrcur] = c;
			tempstrcur++;	
		}	
		else  //tempstr doesn't have enough space to hold the characters from the line
		{	
			HANDLE_REALLOC(tempstr,tempstrmax+50);
			tempstrmax+=50;
			tempstr[tempstrcur]=c;
			tempstrcur++;
			
		}
		

	}
	
} 

line * findline(int l) //returns a pointer to the (structure of) line number passed
{
	line *temp;
	if(l==1)
		return buffer;
	if(l > lastline->thisline)
		return NULL;
	for(temp=buffer;(temp->thisline) < l;temp=temp->next);
		return temp;
}


void buildbuffertill(int l) 
{
	if (l<=(lastline->thisline))
		return;	
	else
	{	int temp=lastline->thisline;
		do
		{
			HANDLE_CALLOC(lastline->next,sizeof(line),1);
			lastline=lastline->next;
			HANDLE_CALLOC(lastline->linestring,INITIAL_CHAR_MAX,sizeof(char));
			lastline->thisline=temp+1;
			temp++;
			
		}	
		while(lastline->thisline < l);
		
		return;
		

	}
}


void allocstring(line *p) //(consider #defining function code)
{
	
	HANDLE_CALLOC(p->linestring,sizeof(char),INITIAL_CHAR_MAX);
	p->maxsize=INITIAL_CHAR_MAX;
	return;
	
}

void expandstring(line *p)  //consider #defining function code
{
	HANDLE_REALLOC(p->linestring,sizeof(char)*((p->maxsize)+50));
	(p->maxsize) += 50;
	return;
}

int delete(int l) 
{
	int templinenum=l;
	line *temp;
	savebeforequit=1;	
	if(l==1)
	{	
		if(lastline->thisline==1)
		{			
			memset(buffer->linestring,'\0',buffer->maxsize);
			return 0;
		}
		line *p=buffer->next;
		for(temp=p;temp->next;temp=temp->next,templinenum++)//Changing the line numbers of the lines after the deleted line
			temp->thisline=templinenum;
		free(buffer->linestring);
		free(buffer);	
		buffer=p;
		return 0;
	}
	else if(l< (lastline->thisline) && l>0)
	{	
		line *p=findline(l-1);
		line *p1= p->next;
		line *p2 = p1->next;
		temp=p2;
		for(;temp->next;temp=temp->next,templinenum++)
			temp->thisline=templinenum;
		free(p1->linestring);
		free(p1);
		(p->next)=p2; //connecting the structures
		
		return 0;
	}
	else if(l==(lastline->thisline))
	{
		line *p=findline(l-1);	
		
		free(lastline->linestring);
		free(lastline);
		p->next = NULL;
		lastline=p;

	}

	else return LINE_ERROR;
	
}


int replace(int l, char *string)
{	
	int i;	
	int stringlength=strlen(string);
	line *p=findline(l);
	savebeforequit=1;
	if(!p)// findline() returns NULL if line hasn't been built yet
	{	buildbuffertill(l);
		p=findline(l);
	}	
	if(!(p->linestring)) // check if string on he line has been allocated memory
		allocstring(p);
	
	while((p->maxsize) < (stringlength + 1))
	{
			expandstring(p);
	}	
	strcpy((*p).linestring,string); 
	return 0;
}

int append(int l, char *string)
{

	line *p=findline(l);
	savebeforequit=1;
	if(!p)// findline() returns NULL if line hasn't been built yet
	{	buildbuffertill(l);
		p=findline(l);	
		replace(l,string);
		return 0;
	}
	else
	{	

		int linelength;
		int stringlength;
		
		if(!(p->linestring))
			allocstring(p);	
		linelength=strlen(p->linestring);
		stringlength=strlen(string);	
		
		while((p->maxsize )< (linelength + stringlength + 1) )
			expandstring(p);
		strcpy((p->linestring)+(linelength),string);
		return 0;
	}
} 

int print()
{	line *temp;
	int line=1;
	putchar('\n');
	printf("-------------------------------------------------\n\t\t%s\n-------------------------------------------------\n",filename);
	for( temp=buffer ;temp!=lastline; temp=(temp->next),line++) // breaks at the last line
	{	printf("% -6d |",line);	
		if(!(temp->linestring))//ignore lines without initialized strings in them
			{putchar('\n'); continue;}
		printf("%s \n",temp->linestring);
	}
	printf("% -6d |",line);
	if(temp->linestring)
		printf("%s \n",temp->linestring);//prints the last line	
	return 0;	
}

int write(FILE **fptr)
{
	char remove[40]="rm ";
	savebeforequit=0;
	line *temp=buffer;
	strcat(remove,filename);
	system(remove); // deleting filestream and get rid of all the previous text
	*fptr=fopen(filename,"w+"); // opening new filestream with same name
	//figure out better way to flush a file
	for( ;temp!=lastline; temp=(temp->next)) // breaks at the last line
	{	if(!(temp->linestring))//ignore lines without initialized strings in them
			{fprintf(*fptr,"\n");
			continue;}
		fprintf(*fptr,"%s\n",temp->linestring);
	}
	if(temp->linestring)
		fprintf(*fptr,"%s\n",temp->linestring);//prints the last line
	return 0;

}


void displayhelp()
{
	printf("%-25s %s","\n\n\'replace l<foo> |bar\'","  to replace the content at line \'foo\' with string \'bar\'\n");
	printf("%-25s %s","\'append l<foo> |bar\'","to add \'bar\' at the end of line \'foo\'\n");
	printf("%-25s %s","\'delete l<foo>\'","to delete line \'foo\'\n");
	printf("%-25s %s","\'write\'","to save the file in the buffer into the file\n");
	printf("%-25s %s","\'print\'","to view the buffer\n");	
	printf("%-25s %s","\'help\'","to display this again\n");	
	printf("%-25s %s","\'quit\'","to end program\n\n"); 
	return;
	
}






void processraw(char *rawin,char *string)
{
	int i,j;
	char *tempcommand=(char *)calloc(sizeof(char),50);
	char *linenumstring=(char *)calloc(sizeof(char),50);
	int templine=0;
	//copying command	
			
	for(i=0;rawin[i]!=' '&&rawin[i]!='\0';i++)
		tempcommand[i]=rawin[i];
	//null character to end string
	tempcommand[i]='\0';
	//comparing strings to determine command
	
	if(!strcmp(tempcommand,"delete"))
		command=DELETE;
	else if(!strcmp(tempcommand,"append"))
		command=APPEND;
	else if(!strcmp(tempcommand,"replace"))
		command= REPLACE;	
	else if(!strcmp(tempcommand,"quit"))
		command= QUIT;
	else if(!strcmp(tempcommand,"print"))
		command= PRINT;
	else if(!strcmp(tempcommand,"write"))
		command= WRITE;
	else if(!strcmp(tempcommand,"help"))
		command= HELP;	
	else
	{	
		command= COMMAND_ERROR;
		return;
	}

	//Determining line number
	if(command==DELETE||command==APPEND||command==REPLACE)
	{
		i++;
		if(rawin[i]=='l')
		{	
			
			i++;			
			for(j=0;rawin[i]!=' '&&rawin[i]!='\0';i++,j++)
				linenumstring[j]=rawin[i];
			linenumstring[++j]='\0';
			templine=atoi(linenumstring);
					
			if(templine>0)
				linenum=templine;
			else
			{
				command=LINE_ERROR;
				return;			
			}		
		}	
		else
			{command=LINE_ERROR; return;}
	}
	//copying input string prefixed with '|'
	if(command==APPEND||command==REPLACE)
	{
		i++;
		if(rawin[i]=='|')
		{
			i++;
			strcpy(string,&rawin[i]);	
		}
		else
			{command = COMMAND_ERROR; return;}
	}
}

int main(int argc, char *argv[])
{
	
	//File Creation
	//add edit already existing file funcionality -- Done
	FILE *fptr;
	char *string=(char *)calloc(sizeof(char),500);
	char *rawin=(char *)calloc(sizeof(char),500);
	
	//create buffer and build first line
	HANDLE_CALLOC(buffer,sizeof(line),1);
	lastline=buffer;
	lastline->thisline=1;
	
	if (argc==1) //no filename given by the user
		printf("\nUsage: ./sit filename\n\n"),exit(1);
	if(argc>2) //Too many arguments enetred by the user
		printf("\nToo many arguments\n\n");
	
	else if(argc==2) //filename received
	{	strcpy(filename,argv[1]);
		if(fptr= (FILE *)fopen(filename,"r")) //check if file with same name exists
			readfileintobuffer(fptr);
		else
		{
			if(! (fptr = (FILE *)fopen(filename,"w")))//open file and check if successful
				printf("Memory error"), exit(1);		

		}
			

	}

	
	


	displayhelp();


	//Ready to receive commands
	while(1)
	{	
			
		printf("\n?\n");
		fflush(stdin);
		gets(rawin);
		processraw(rawin,string);
		

		
		switch (command)
		{
			case QUIT: 	if(savebeforequit)
					{
						char c;
						printf("The buffer has been changed since the last save,  save now? \n[y or n]? :\t");

						scanf("%c",&c);
						if(c=='y' || c=='Y')
							write(&fptr);
					}
					exit (0);
					break;
				   		
			case REPLACE:   replace(linenum,string);
					break;
			case APPEND:    append(linenum,string);
					break;
			case WRITE:     write(&fptr);
					break;
			case PRINT:     print();
					break;
			case DELETE:    if(delete(linenum)==LINE_ERROR)
						printf("Invalid line number");
					break;
			case COMMAND_ERROR: printf("Command Error, type 'help' for help");
						break;
			case LINE_ERROR: printf("Improper line number");
					break;
			case HELP: displayhelp();
					break;
			default: printf("Command Error");
 			 		

		}
		
		command = 0;
		linenum =0;
		memset(rawin,'\0',500);
		memset(string,'\0',500);
		
		
			
	}
	
}
