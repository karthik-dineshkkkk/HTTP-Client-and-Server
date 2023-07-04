
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include<fcntl.h>
#include<sys/stat.h>

			/* THE SERVER PROCESS */

#define PORT 20002
#define BUFSIZE 1000
#define WEBROOT "./webroot"

typedef struct {
  char host[BUFSIZE];
  char date[BUFSIZE];
  char user_agent[BUFSIZE];
  char accept[BUFSIZE];
  char accept_language[BUFSIZE];
  char content_language[BUFSIZE];                  // redundant probably
  char content_type[BUFSIZE];                       // redundant
  int content_length;                               // redundant
  char if_modified_since[BUFSIZE];
  char connection[BUFSIZE];
} GET_REQ;

typedef struct {
  char host[BUFSIZE];
  char date[BUFSIZE];
  char user_agent[BUFSIZE];
  char accept[BUFSIZE];
  char accept_language[BUFSIZE];
  char content_language[BUFSIZE];
  char content_type[BUFSIZE];
  int content_length;
  char if_modified_since[BUFSIZE];
  char connection[BUFSIZE];
} PUT_REQ;


typedef struct {
  char expires[BUFSIZE];
  char cache_control[BUFSIZE];
  char content_type[BUFSIZE];
  int content_length;
  char content_language[BUFSIZE];
  char last_modified[BUFSIZE];

} PUT_RES;

typedef struct {
  char expires[BUFSIZE];
  char cache_control[BUFSIZE];
  char content_type[BUFSIZE];
  int content_length;
  char content_language[BUFSIZE];
  char last_modified[BUFSIZE];

} GET_RES;

char *get_file_path(char *url) {
char *file_path = malloc(strlen(url) + strlen(WEBROOT) + 1);
strcpy(file_path, WEBROOT);
strcat(file_path, url);
return file_path;
}


int check_file_exists(char* filepath) {

    if( access( filepath, F_OK ) != -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
}

void error_code(int error_code) {
  char error_message[100];
  switch (error_code) {
    case 400:
      sprintf(error_message, "ERROR: 400 Bad Request\n\n");
      break;
    case 403:
      sprintf(error_message, "ERROR: 403 Forbidden\n\n");
      break;
    case 404:
      sprintf(error_message, "ERROR: 404 Not Found\n\n");
      break;
    default:
      sprintf(error_message, "ERROR: 500 Internal Server Error\n\n");
  }
  // send(newsockfd, error_message, strlen(error_message), 0);
  
}

char *get_file_type(char *url) {
  char *dot = strrchr(url, '.');
  if (!dot || dot == url) return "text/plain";
  
  if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html";
  if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
  if (strcmp(dot, ".png") == 0) return "image/png";
  if (strcmp(dot, ".gif") == 0) return "image/gif";
  if (strcmp(dot, ".css") == 0) return "text/css";
  if (strcmp(dot, ".js") == 0) return "application/javascript";
  
  return "text/plain";
}

void access_log(char *client_ip, int client_port, char buf[], char met[]) {
        char* field[BUFSIZE];
         char* token;
         char url[100];
        //  printf("\nAccess log buf: %s",buf);
        token = strtok(buf," ");
       int i=0;
        while (token != NULL) {
        field[i++] = token;
        token = strtok(NULL, " ");
        }

        strcpy(url,field[1]);
        // printf("Access log url: %s\n",url);
    time_t t;
    struct tm *present_time;
    char str[40];
    char string[200];

    time(&t);
    present_time = localtime(&t);
    strftime(str, sizeof(str), "%d-%m-%y:%H.%M.%S", present_time);

    sprintf(string, "%s  :%s :%d :%s :%s \n", str, client_ip, client_port,url,met);

    FILE *fp = fopen("file.txt", "a");
    
        fputs(string, fp);
        fclose(fp);
    
}

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;
  struct dirent *pDirent;
  DIR *pDir;
  char header_content[BUFSIZE],status_message[100];

    char buf[BUFSIZE],buf2[BUFSIZE];
    int status_code;
    

	int i,j,word;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); 
	
	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		
		// get client ip and port
	  char *client_ip = inet_ntoa(cli_addr.sin_addr);
    int client_port = ntohs(cli_addr.sin_port);

		printf("Client IP: %s\n", client_ip);
    printf("Client Port: %d\n", client_port);


		if (fork() == 0) {

			close(sockfd);

      // Recieve data as http protocol
			for(i=0; i < 100; i++) buf[i] = '\0';           // recieve user name
			int x = recv(newsockfd, buf, sizeof(buf), 0);           // recieve iteratively -> change later, stotr in user_name
			
      printf("Recieved: %s",buf);

      // Process the header data
      char name[BUFSIZE], value[BUFSIZE]; 
      GET_REQ get_req;
      PUT_REQ put_req;
      char information[strlen(buf)+1];

      int i;
      // read buf to first line
      char firstline[BUFSIZE];
      for(i=0;i<strlen(buf);i++) {
          if(buf[i] != '\n' && buf[i] != '\0')
              firstline[i] = buf[i];
          else {
              firstline[i] = '\0';
              break;
          }
      }
      printf("First Line: %s\n",firstline);

      char method[100],file_path[BUFSIZE],version[100];

      int j = 0, k = 0;
      for (i = 0; firstline[i] != ' '; i++)
      method[i] = firstline[i];
      method[i] = '\0';

      i++;
      if (firstline[i] == '/')
      {
        i++;
      }
      
      for (; firstline[i] != ' '; i++)
      file_path[j++] = firstline[i];
      file_path[j] = '\0';

      i++;
      for (; i < strlen(firstline); i++)
      version[k++] = firstline[i];
      version[k] = '\0';

      // char *delimiter = " ";
      // method = strtok(firstline," ");
      // char *temp = strtok(NULL, "\n");
      // file_path = temp +strlen(delimiter);
      // version = strrchr(file_path, ' ');
      // if (version != NULL) {
      //     *version = '\0';
      //     version = version + strlen(delimiter);
      // } else {
      //     version = "";
      // }

      

      printf("method: %s,file_path: %s, version: %s\n",method,file_path,version);
      if(strcmp(method,"PUT")==0) {
      //  printf("PUT FUNC RUNNING...\n");
      }
      if(strcmp(method,"GET")==0) {
      //  printf("GET FUNC RUNNING...\n");
      }
      // Take headers and content of request/response
      j = 0;
      k = 0;
      int blank_line = 0;
      i=0;
      while (buf[i] != '\0') {
        j = 0;
        while (buf[i] != '\n') {
          if (buf[i] == ':') {
            name[j] = '\0';
            j = 0;
            i++;
            while (buf[i] == ' ') {
              i++;
            }
            while (buf[i] != '\n') {
              value[j++] = buf[i++];
            }
            value[j] = '\0';
            // printf("Name: %s\nValue: %s\n", name, value);
          
            break;
          }
          name[j++] = buf[i++];
        }
        i++;

        if (strcmp(method,"GET")==0) {
            if (strcmp(name,"Host")==0) {
            // printf("HOst here\n");
            strcpy(get_req.host,value);
            printf("get req host: %s\n",get_req.host);
          } else if (strcmp(name,"Connection")==0){
            strcpy(get_req.connection,value);
            printf("get req connection: %s\n",get_req.connection);
          } else if (strcmp(name,"Accept")==0){
            strcpy(get_req.accept,value);
            printf("get req Accept: %s\n",get_req.accept);
          } else if (strcmp(name,"Accept-Language")==0) {
            strcpy(get_req.accept_language,value);
            printf("get req Accept Language: %s\n",get_req.accept_language);
          } else if (strcmp(name, "date") == 0) {
                strcpy(get_req.date, value);
                printf("get req Date: %s\n",get_req.date);
          } else if (strcmp(name, "if-modified-since") == 0) {
                strcpy(get_req.if_modified_since, value);
                printf("get req if-modified-since: %s",get_req.if_modified_since);
          } else if (strcmp(name,"User-Agent")==0) {
            strcpy(get_req.user_agent,value);
            printf("get req User-Agent: %s\n",get_req.user_agent);
          }else if (strcmp(name,"Content-Language")==0) {
            strcpy(get_req.content_language,value);
            printf("get req Content-Language: %s\n",get_req.content_language);
          }else if (strcmp(name,"Content-type")==0) {
            strcpy(get_req.content_type,value);
            printf("get req Content-type: %s\n",get_req.content_type);
          }
        }
        else if(strcmp(method,"PUT")==0) {
          if (strcmp(name,"Host")==0) {
              // printf("HOst here\n");
              strcpy(put_req.host,value);
              printf("put req host: %s\n",put_req.host);
            } else if (strcmp(name,"Connection")==0){
              strcpy(put_req.connection,value);
              printf("put req connection: %s\n",put_req.connection);
            } else if (strcmp(name,"Accept")==0){
              strcpy(put_req.accept,value);
              printf("put req Accept: %s\n",put_req.accept);
            } else if (strcmp(name,"Accept-Language")==0) {
              strcpy(put_req.accept_language,value);
              printf("put req Accept Language: %s\n",put_req.accept_language);
            } else if (strcmp(name, "date") == 0) {
                  strcpy(put_req.date, value);
                  printf("put req Date: %s\n",put_req.date);
            } else if (strcmp(name, "if-modified-since") == 0) {
                  strcpy(put_req.if_modified_since, value);
                  printf("put req if-modified-since: %s",put_req.if_modified_since);
            } else if (strcmp(name,"User-Agent")==0) {
              strcpy(put_req.user_agent,value);
              printf("put req User-Agent: %s\n",put_req.user_agent);
            } else if (strcmp(name,"Content-Language")==0) {
            strcpy(get_req.content_language,value);
            printf("put req Content-Language: %s\n",get_req.content_language);
          } else if (strcmp(name,"Content-type")==0) {
            strcpy(get_req.content_type,value);
            printf("put req Content-type: %s\n",get_req.content_type);
          }
          
        }

          if (buf[i] == '\n') {
            blank_line = 1;
            break;
          }
          
      }

      if (blank_line) {
        i++;
        while (buf[i] != '\0') {
          information[k++] = buf[i++];
        }
        information[k] = '\0';
       // printf("Information: %s\n", information);
      }

      // Process command
      time_t t;
        struct tm *present_time,three_days_later;
        char date[40],expiry_date[40];
        time(&t);
        present_time = localtime(&t);
        three_days_later = *present_time;
        three_days_later.tm_mday += 3;
        mktime(&three_days_later);
        strftime(expiry_date, sizeof(expiry_date), "%d-%m-%Y", &three_days_later);
        strftime(date, sizeof(date), "%d-%m-%y", present_time);

      if ( strcmp(method,"GET") == 0){
           printf("GET FUNC RUNNING...\n");

          // strcpy(met,field[0]);
            access_log(client_ip, client_port,buf, method);
          
          printf("File path: %s\n",file_path);
          if (!file_path) {
            error_code(400);
            status_code = 400;
            strcpy(status_message,"ERROR: 400 Bad Request");
          }
          else {

            // Check if file exists
           // printf("File path in server: %s",file_path);
            if (check_file_exists(file_path)==0) {
              error_code( 404);
              status_code = 404;
              strcpy(status_message,"ERROR: 404 File Not Found");
            } else {
              //Get the file type based on the extension of the URL
              char *file_type = get_file_type(file_path);
              FILE *file = fopen(file_path, "r");
              if (file == NULL) {
                  error_code(403);
                  status_code = 403;
                  strcpy(status_message,"Access Forbidden");
              }
              else {
                status_code = 200;
                strcpy(status_message,"OK");
                // printf("File present and permitted to read\n");
                // file is present and peritted to read
                
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                rewind(file);

                char *file_contents = malloc(file_size + 1);
                if (file_contents == NULL) {
                printf("Error allocating memory: %s\n", strerror(errno));
                return 1;
                }

                // Read the file into the file_contents buffer
                size_t bytes_read = fread(file_contents, 1, file_size, file);
                if (bytes_read != file_size) {
                  printf("Error reading file: %s\n", strerror(errno));
                  return 1;
                }

                  // Add a null terminator to the end of the string
                file_contents[file_size] = '\0';
               // printf("File content: %s\n",file_contents);
          
                fclose(file);
                
                // Set value of header content
                sprintf(header_content,"HTTP/1.1 %d %s\nDate: %s\nServer: Apache/2.4.41\nContent-length: %ld\nContent-type: %s\nContent-language: en-US\nExpires: %s\nLast modified: %s\nCache-control: no-store",status_code,status_message,date,file_size,file_type,expiry_date,date);

                // Allocate memory for the combined string
                char *combined_string = malloc(file_size + strlen(header_content) + 1);
                if (combined_string == NULL) {
                    printf("Error allocating memory: %s\n", strerror(errno));
                    return 1;
                }

                // Concatenate the file contents and the additional content
                strcpy(combined_string, header_content);
                strcat(combined_string,"\n\n");
                strcat(combined_string, file_contents);

                // Print the combined string
                // printf("Combined String: %s\n", combined_string);
                //send(newsockfd, combined_string, strlen(combined_string) + 1, 0);
                // Free the memory
                char str[strlen(combined_string)+1];
                strcpy(str,combined_string);
                str[strlen(combined_string)] = '\0';
                send(newsockfd, str, strlen(str) + 1, 0);
               // printf("Combined String: %s\n",str);
                free(file_contents);
                free(combined_string);

              
                //send(newsockfd, str, strlen(str) + 1, 0);
              }
            }
          }

          // Check for status codes and send response accordingly
          // printf("Status code: %d",status_code);
          if (status_code != 200) {
              // Set value of header content
              sprintf(header_content,"HTTP/1.1 %d %s\nDate: %s\nServer: Apache/2.4.41\nContent-length: 0\nContent-type: None\nContent-language: en-US\nExpires: %s\nLast modified: %s\nCache-control: no-store\n\nsoodi",status_code,status_message,date,expiry_date,date);
              send(newsockfd,header_content,strlen(header_content)+1,0);
              printf("status code: %d, Status message: %s",status_code,status_message);
          }
			}
             
      else if (strcmp(method,"PUT") == 0){
      
        access_log(client_ip, client_port,buf,method);
        status_code = 200;
        strcpy(status_message,"OK");
        // create the full path of the file
        char full_path[BUFSIZE];
        strcpy(full_path, file_path);
        // strcat(full_path, "/");
        // strcat(full_path, filename);

        // check if file already exists
        if (access(full_path, F_OK) != -1)
        {
            printf("File Path already exists\n");

            // check if we have write permission
            if (access(full_path, W_OK) == 0)
            {
                // open the file in write mode
                int fd = open(full_path, O_RDWR | O_TRUNC);
                if (fd == -1)
                {
                    perror("Error opening file");
                    status_code = 403;
                    strcpy(status_message,"Forbidden access");
                } else {

                    // write the content to the file
                    int bytes_written = write(fd, information, strlen(information));
                    if (bytes_written == -1)
                    {
                        perror("Error writing to file");
                        status_code = 403;
                        strcpy(status_message,"Forbidden access");
                    } else {
                        
                        printf("File content updated successfully\n");
                        status_code = 200;
                        strcpy(status_message,"OK");
                    }
                    // close the file
                    close(fd);
                }
            }
            else
            {
                printf("No write permission\n");
                status_code = 403;
                strcpy(status_message,"Forbidden access");
            }
        }
        else{
                       if (strcmp(get_req.content_type, "pdf") == 0) {

                      status_code = 200;
                strcpy(status_message,"OK");
                printf("File created successfully\n");

                       }else{


            int fd = open(full_path, O_CREAT | O_RDWR, S_IRWXU);
            if (fd == -1)
            {
                perror("Error creating file");
                // return -1;
                status_code = 400;
                strcpy(status_message,"Bad Request");
            } 
            else {
            // write the content to the file
            int bytes_written = write(fd, information, strlen(information));
            if (bytes_written == -1)
            {
                perror("Error writing to file");
                status_code = 403;
                strcpy(status_message,"Forbidden access");
            } else {
                status_code = 200;
                strcpy(status_message,"OK");
                printf("File created successfully\n");
            }
            // close the file
            close(fd);
            }
        }
      }
        // Send http response to the client after finding appropriate status code and message
        sprintf(header_content,"HTTP/1.1 %d %s\nDate: %s\nServer: Apache/2.4.41\nExpires: %s\nLast modified: %s\nCache-control: no-store\n\n",status_code,status_message,date,expiry_date,date);
        header_content[strlen(header_content)] = '\0';
        send(newsockfd,header_content,strlen(header_content)+1,0);
        printf("\nHttp response: %s",header_content);
      }
			
			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
	return 0;
}

// strtok