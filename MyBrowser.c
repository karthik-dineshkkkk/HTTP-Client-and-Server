
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stddef.h>
#include <time.h>
#include<errno.h>

#define DEFAULT_PORT 20002
#define BUFSIZE 2000

typedef struct {
  char expires[BUFSIZE];
  char cache_control[BUFSIZE];
  char last_modified[BUFSIZE];
  char server[BUFSIZE];
  char date[100];
} PUT_RES;

typedef struct {
  char expires[BUFSIZE];
  char cache_control[BUFSIZE];
  char content_type[BUFSIZE];
  int content_length;
  char content_language[BUFSIZE];
  char last_modified[BUFSIZE];
  char server[BUFSIZE];
  char date[100];
} GET_RES;

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

int port_number(char url[])
{
    
    int i, len,port = 0;
   
    len = strlen(url);
    for (i = 7; i < len; i++)
    {
        if (url[i] == ':')
        {
            for (i = i + 1; i < len; i++)
            {
                 if (url[i] == '/')
                   {
                   port = 0;
                   break;
                   } 
                port = port * 10 + (url[i] - '0');
            }
            break;
        }
    }
    printf("Port: %d", port);
    if (port == 0) return DEFAULT_PORT;
    else return port;
}


void me_ur_pr(char buf[]){

    char *token;
    char method[10];
    char url[100];
    char url1[100];
    char protocol_version[10];
    
    //char *q;
    char buf4[BUFSIZE];
    strcpy(buf4,buf);
    token = strtok(buf, " ");
    strcpy(method, token);

    token = strtok(NULL, " ");
    strcpy(url, token);

    char *path_to_file = strstr(url, "//");
    path_to_file = strchr(path_to_file + 2, '/');
  /*  if (strstr(url, "http://")) {
        path_to_file = path_to_file + 7;
    }*/
   // strcpy(path_to_file,q);
  // path_to_file = strstr(url, "/");

    token = strtok(NULL, " ");
    strcpy(protocol_version, token);

    printf("Method: %s\n", method);
    printf("URL: %s\n", path_to_file);
    printf("Protocol Version: %s\n", protocol_version);
}

int main(int argc, char *argv[]) {

    int client_socket;
    struct sockaddr_in serv_addr;
    int i,j;
    char command[BUFSIZE],buf[BUFSIZE],buf2[BUFSIZE],buf3[BUFSIZE],response[BUFSIZE];;
    char url[BUFSIZE];
    char filename[BUFSIZE];
    char* field[BUFSIZE];
    char* token;
    char http_version[100], str_status_code[100], Status_message[100];

    // client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // find server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DEFAULT_PORT);
    // inet_pton(AF_INET, url, &serv_addr.sin_addr);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
   
    while (1) {

        printf("MyOwnBrowser> ");
        for(i=0;i<sizeof(buf);i++) buf[i]='\0';
        fgets(buf, BUFSIZE, stdin);

        printf("After fgets buf: %s\n",buf);

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

        char method[100],Input_url[BUFSIZE],Input_file_optional[BUFSIZE];

        int j = 0, k = 0;
        for (i = 0; firstline[i] != ' '; i++)
        method[i] = firstline[i];
        method[i] = '\0';

        i++;
        if (strcmp(method,"PUT")==0) {
            printf("PUT Method\n");
            for (; firstline[i] != ' '; i++)
            Input_url[j++] = firstline[i];
            Input_url[j] = '\0';

            i++;
            for (; i < strlen(firstline) && firstline[i]!='\n'; i++)
            Input_file_optional[k++] = firstline[i];
            Input_file_optional[k] = '\0';
        }
        else if (strcmp(method,"GET")==0) {
            printf("GET Method\n");
            for (; i < strlen(firstline) && firstline[i]!='\n'; i++)
            Input_url[k++] = firstline[i];
            Input_url[k] = '\0';
        }


       
        printf("Method: %s\nUrl: %s\nInput_file_optional: %s\n",method,Input_url,Input_file_optional);

        // calculate date, if_modified_since -> 2 days before
        time_t t;
        struct tm *present_time,two_days_ago;
        char date[40],modified_since_date[40];
        time(&t);
        present_time = localtime(&t);
        two_days_ago = *present_time;
        two_days_ago.tm_mday -= 2;
        mktime(&two_days_ago);
        strftime(modified_since_date, sizeof(modified_since_date), "%d-%m-%Y", &two_days_ago);
        strftime(date, sizeof(date), "%d-%m-%y", present_time);

        // get port number
        // char str[200];
        // strcpy(str,field[1]);
        int x = port_number(Input_url);
        printf(" port number is %d\n",x);
        // set port number
        serv_addr.sin_port = htons(x);
        
        // if command is GET
        if ( strcmp(method,"GET") == 0){
            printf("GET commad identified\n");
            client_socket = socket(AF_INET, SOCK_STREAM, 0);
            if ((connect(client_socket, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {
                perror("Unable to connect to server\n");
                exit(0);
            }
            // strcpy(buf3,buf);
            // me_ur_pr(buf3);

            // get file type
            char *file_type; 
            file_type = strrchr(Input_url, '.');
            file_type++; 
            // if port exists
            file_type = strtok(file_type,":");
            printf("File type: %s\n", file_type);

            // get file_path
            char buf4[BUFSIZE];
            strcpy(buf4,buf);
            token = strtok(buf, " ");
            token = strtok(NULL, "\n");
            strcpy(url, token);
            char *path_to_file = strstr(url, "//");
            char host[200];
            sscanf(url,"http://%[^/]",host);
            printf("host: %s\n",host);
            path_to_file = strchr(path_to_file + 2, '/');

            // if port exists
            path_to_file = strtok(path_to_file,":");
            printf("Path to File in server: %s\n", path_to_file);

            // formatting and sending data to send for server
            // char *data = "PUT /path/to/file.pdf HTTP/1.1\nHost: www.example.com\nUser-Agent: MyClient/1.0\nAccept: application/pdf\nAccept-Language: eng-us\nConnection: keep-alive\n\nrandomtestsld;fhglsdfghzdjksf mxnksmx";
            char data[BUFSIZE];
            sprintf(data,"GET %s HTTP/1.1\nHost: %s\nConnection: close\nDate: %s\nAccept: %s\nAccept-Language: en-US, en:q=0.9\nIf-modified-since: %s\nContent-Language: en-US, en:q=0.9\nUser-Agent: Mozilla/98.0.1\nContent-type: %s\n\ngetdemocontent justtotest",path_to_file,host,date,file_type,modified_since_date,file_type);
	        
            send(client_socket, data, strlen(data) + 1, 0);                           // send to server iteratively
            
            // recieve http response
            for(i=0;i<=sizeof(buf);i++) buf[i] = '\0';
            int l = recv(client_socket, buf, sizeof(buf), 0);

            printf("Response: %s\n", buf);	

            // Parse headers and data of file

            // Process the header data
            char name[BUFSIZE], value[BUFSIZE]; 
            GET_RES get_res;
            PUT_RES put_res;
            char information[strlen(buf)+1];
            int i;
        
            
            
            if(strcmp(method,"GET")==0) {
                printf("GET FUNC RUNNING...\n");
            }
            // Take headers and content of request/response
            int j = 0, k = 0, blank_line = 0;
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
                    if (strcmp(name,"Expires")==0) {
                    // printf("HOst here\n");
                    strcpy(get_res.expires,value);
                    printf("get res expires: %s\n",get_res.expires);
                } else if (strcmp(name,"Cache-control")==0){
                    strcpy(get_res.cache_control,value);
                    printf("get res cache_control: %s\n",get_res.cache_control);
                }else if (strcmp(name,"Content-language")==0) {
                    strcpy(get_res.content_language,value);
                    printf("get res Content-language: %s\n",get_res.content_language);
                }else if (strcmp(name,"Content-type")==0) {
                    strcpy(get_res.content_type,value);
                    printf("get res Content-type: %s\n",get_res.content_type);
                } else if (strcmp(name,"Content-length")==0){
                    get_res.content_length = atoi(value);
                    printf("get res Content-length: %d\n",get_res.content_length);   
                } else if (strcmp(name,"Last modified")==0) {
                    strcpy(get_res.last_modified,value);
                    printf("get res Last modified: %s\n",get_res.last_modified);                    
                } else if (strcmp(name,"Server")==0) {
                    strcpy(get_res.server,value);
                    printf("get res Server: %s\n",get_res.server);
                } else if (strcmp(name,"Date")==0) {
                    strcpy(get_res.date,value);
                    printf("get res Date: %s\n",get_res.date);
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
                printf("File content information GET: %s\n", information);
            }

            // check status codes and status message
            // read buf to first line
            
            for(i=0;i<strlen(buf);i++) {
                if(buf[i] != '\n' && buf[i] != '\0')
                    firstline[i] = buf[i];
                else {
                    firstline[i] = '\0';
                    break;
                }
            }
            printf("First line: %s\n",firstline);
            j = 0, k = 0;
            for (i = 0; firstline[i] != ' '; i++)
            http_version[i] = firstline[i];
            http_version[i] = '\0';

            i++;
            for (; firstline[i] != ' '; i++)
            str_status_code[j++] = firstline[i];
            str_status_code[j] = '\0';

            i++;
            for (; i < strlen(firstline); i++)
            Status_message[k++] = firstline[i];
            Status_message[k] = '\0';
        
            printf("GET Status code: %s, status message: %s\n",str_status_code,Status_message);

            // Store contents of file to file
            if (strcmp(str_status_code,"200")==0) {
                // create file
                char file_name[BUFSIZE];
                strcpy(file_name,"Ass4");
                
                strcat(file_name, ".");
                strcat(file_name, file_type);
                printf("%s",file_name);


                 if (strcmp(file_type, "pdf") == 0) {
        //    FILE *file = fopen(file_name, "wb");
        //    if (file == NULL) {
        //     printf("Error opening file!\n");
        //     return 1;
        //     }
        //     fwrite(response, sizeof(char), strlen(response), file);
        //     fclose(file);
                 char cwd[1024];     
                 char input_path[1024];
                  if (getcwd(cwd, sizeof(cwd)) == NULL) {
               printf("Error:\n");
                  return 1;
                }
                strcpy(input_path, cwd);
                strcat(input_path, "/");
                strcat(input_path, path_to_file);

                FILE *input = fopen(input_path, "rb");
    if (input == NULL) {
        printf("Error: %s\n", input_path);
        return 1;
    }

    // Construct the absolute path of the output file
    char output_path[1024];
    strcpy(output_path, cwd);
    strcat(output_path, "/Ass4.pdf");

    // Open the output file
    FILE *output = fopen(output_path, "wb");
    if (output == NULL) {
        printf("Error: Unable to open output file %s\n", output_path);
        fclose(input);
        return 1;
    }

    // Copy the contents of the input file to the output file
    char buffer[1024];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, n, output) != n) {
            printf("Error: Unable to write to output file %s\n", output_path);
            fclose(input);
            fclose(output);
            return 1;
        }
    }

    
    fclose(input);
    fclose(output);

            } else {

                FILE *file = fopen(file_name, "w");
                if (file == NULL) {
                printf("Error opening file!\n");
                }
    
                fprintf(file, "%s", information);
                fclose(file);
            } }else {
                printf("GET Status code: %s, status message: %s\n",str_status_code,Status_message);
                printf("%s\n",Status_message);
            }

            close(client_socket);
        }
        else if (strcmp(method,"PUT") == 0){
            printf("PUT commad identified\n");
            // connect to server, 
            client_socket = socket(AF_INET, SOCK_STREAM, 0);
            if ((connect(client_socket, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {
                perror("Unable to connect to server\n");
                exit(0);
            }

            // get host, path, file_name, port
            strcpy(filename,Input_file_optional);
            printf("File name: %s",filename);
            // get file_type
            char *file_type; 
            file_type = strrchr(filename, '.');
            file_type++; 
            // if port exists
            file_type = strtok(file_type,":");
            printf("File type: %s\n", file_type);
            // get url
            char buf4[BUFSIZE];
            strcpy(buf4,buf);
            token = strtok(buf, " ");
            token = strtok(NULL, " ");
            strcpy(url, token);
            char *path_to_file = strstr(url, "//");
            char host[200];
            sscanf(url,"http://%[^/]",host);
            printf("host: %s\n",host);
            path_to_file = strchr(path_to_file + 2, '/');

            printf("File path: %s\n",path_to_file);
            
            // Store headers to data
            char data[BUFSIZE];
            sprintf(data,"PUT %s HTTP/1.1\nHost: %s\nConnection: close\nDate: %s\nAccept: %s\nAccept-Language: en-US, en:q=0.9\nIf-modified-since: %s\nContent-Language: en-US, en:q=0.9\nUser-Agent: Mozilla/98.0.1\nContent-type: %s",path_to_file,host,date,file_type,modified_since_date,file_type);

            // get data from file
            if (check_file_exists(filename)==0) {
            printf("ERROR: No such file %s, exists in your system\n",filename);
            continue;
            }

            //Get the file type based on the extension of the URL
             if (strcmp(file_type, "pdf") == 0) {

                /* if (argc != 1)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }*/

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        printf("Error: Unable to get the current working directory\n");
        return 1;
    }
    //char *filename = "output.pdf";
    // char tc[10];
    // strcpy (tc,"/");
    // strcat(tc,filename);
    // strcpy(filename,tc);
    //Construct the absolute path of the input file
    char input_path[1024];
    strcpy(input_path, cwd);
    strcat(input_path, path_to_file);
    //strcat(input_path, argv[1]);

    // Open the input file
     FILE *input = fopen(input_path, "rb");
     input = fopen(input_path, "wb");
    if (input == NULL)
    {
        printf("Error: Unable to open input file %s\n", input_path);
        return 1;
    }

    // Construct the absolute path of the output file
    char output_path[1024];
    strcpy(output_path, cwd);
    strcat(output_path,"/");
    strcat(output_path, filename);

    // Open the output file
    FILE *output = fopen(output_path, "rb");
    if (output == NULL)
    {
        printf("Error: Unable to open output file %s\n", output_path);
        fclose(input);
        return 1;
    }

    char file_contents[20000];
    size_t bytes_read;
    int file_size;
    while ((bytes_read = fread(file_contents, 1, 20000, output)) > 0)
    {
        printf("Bytes read: %ld\n", bytes_read);
         file_size +=bytes_read;
        size_t bytes_written = fwrite(file_contents, 1, bytes_read, input);
        printf("Bytes written: %ld\n", bytes_written);
        if (bytes_written != bytes_read)
        {
            printf("Error: Not all bytes were written to output file\n");
            fclose(output);
            fclose(input);
            return 1;
        }
    }
    
   
    fclose(input);
    fclose(output);

   char *combined_string = malloc(file_size + strlen(data) + 1);
            if (combined_string == NULL) {
                printf("Error allocating memory: %s\n", strerror(errno));
                return 1;
            }

            // Concatenate the file contents and the additional content
            strcpy(combined_string, data);
            strcat(combined_string,"\n\n");
            strcat(combined_string, file_contents);

            // send the combined string and free mallocs
            char str[2000];
            strcpy(str,combined_string);
            send(client_socket, str, strlen(str) + 1, 0);
            printf("Combined String: %s\n",str);
           // free(file_contents);
            free(combined_string);

             }  else{
            FILE *file = fopen(filename, "r");
            
            if (file == NULL) {
                printf("Your file has no read permission\n");
                continue;
            }

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
        
            fclose(file);
                

            // Allocate memory for the combined string
            char *combined_string = malloc(file_size + strlen(data) + 1);
            if (combined_string == NULL) {
                printf("Error allocating memory: %s\n", strerror(errno));
                return 1;
            }

            // Concatenate the file contents and the additional content
            strcpy(combined_string, data);
            strcat(combined_string,"\n\n");
            strcat(combined_string, file_contents);

            // send the combined string and free mallocs
            char str[2000];
            strcpy(str,combined_string);
            send(client_socket, str, strlen(str) + 1, 0);
            printf("Combined String: %s\n",str);
            free(file_contents);
            free(combined_string);
             }
            // recieve http response
            for(i=0;i<=sizeof(buf);i++) buf[i] = '\0';
            int l = recv(client_socket, buf, sizeof(buf), 0);

            printf("Response: %s\n", buf);	

            // Parse the response
            // Process the header data
            char name[BUFSIZE], value[BUFSIZE]; 
            GET_RES get_res;
            PUT_RES put_res;
            char information[strlen(buf)+5];

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
            printf("Response First Line: %s\n",firstline);


            if(strcmp(method,"PUT")==0) {
                printf("PUT FUNC RUNNING...\n");
            }
            
            // Take headers and content of request/response
            int j = 0, k = 0, blank_line = 0;
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
                    printf("Name: %s\nValue: %s\n", name, value);
                
                    break;
                }
                name[j++] = buf[i++];
                }
                i++;

                // Store headers in put_res struct
                
                if (strcmp(name,"Expires")==0) {
                    // printf("HOst here\n");
                    strcpy(put_res.expires,value);
                    printf("put res expires: %s\n",put_res.expires);
                } else if (strcmp(name,"Cache-control")==0){
                    strcpy(put_res.cache_control,value);
                    printf("put res cache_control: %s\n",put_res.cache_control);
                
                } else if (strcmp(name,"Last modified")==0) {
                    strcpy(put_res.last_modified,value);
                    printf("put res Last modified: %s\n",put_res.last_modified);                    
                } else if (strcmp(name,"Server")==0) {
                    strcpy(put_res.server,value);
                    printf("put res Server: %s\n",put_res.server);
                } else if (strcmp(name,"Date")==0) {
                    strcpy(put_res.date,value);
                    printf("put res Date: %s\n",put_res.date);
                }
                

                if (buf[i] == '\n') {
                    blank_line = 1;
                    printf("Broke loop without problem due to new line\n");
                    break;
                }
                
            }

            if (blank_line) {
                i++;
                while (buf[i] != '\0') {
                information[k++] = buf[i++];
                }
                information[k] = '\0';
                printf("File content information PUT: %s\n", information);
            }
            // Check the status codes, and print status meessage
            // read buf to first line
            
            // for(i=0;i<strlen(buf);i++) {
            //     if(buf[i] != '\n' && buf[i] != '\0')
            //         firstline[i] = buf[i];
            //     else {
            //         firstline[i] = '\0';
            //         break;
            //     }
            // }
        
            for(i=0;i<strlen(buf);i++) {
                if(buf[i] != '\n' && buf[i] != '\0')
                    firstline[i] = buf[i];
                else {
                    firstline[i] = '\0';
                    break;
                }
            }
            printf("First line: %s\n",firstline);
            j = 0;
            k = 0;
            for (i = 0; firstline[i] != ' '; i++)
            http_version[i] = firstline[i];
            http_version[i] = '\0';
            printf("http_version: %s",http_version);
            i++;
            for (; firstline[i] != ' '; i++)
            str_status_code[j++] = firstline[i];
            str_status_code[j] = '\0';
            printf("status code: %s\n",str_status_code);
            i++;
            for (; i < strlen(firstline); i++)
            Status_message[k++] = firstline[i];
            Status_message[k] = '\0';

        
            printf("PUT Status code: %s, status message: %s\n",str_status_code,Status_message);
            // close socket
            close(client_socket);

        }
        else if (strcmp(method,"QUIT") == 0){
            printf("EXIT");
            exit(0);
        }
        else {
            printf("Invalid command\n");
        }

        
    }
   
    return 0;
}
