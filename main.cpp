#include <iostream>
#include <cstring>
#include <unistd.h>
#include <limits>
#include <sstream>
#include <arpa/inet.h>
using namespace std;

#define PORT 8080 //sets the port value ahead of time instead of getting it froom user
#define BUFFER_SIZE 1024

int main() {
    int sock = 0; //socket descriptor
    int input = 0;
    string line1;
    sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0); //creates the socket
    serv_addr.sin_family = AF_INET; //prepares the server address structure
    serv_addr.sin_port = htons(PORT); //sets the port number

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); //converts IP for connect function

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); //connects to server

    read(sock, buffer, BUFFER_SIZE); //reads server connection message
    cout << buffer << endl;

    while(input != 6) //loop for the user
    {
        cout << "(1) Search a movie\n(2) Top rated movies\n(3) Most reviewed movie\n(4) Get reviews for a movie\n(5) Submit a review\n(6) Terminate\nSelect an option [1,2,3,4,5, or 6]: ";
        if (!(cin >> input)) //had a bug where if i put a non integer into the prompt it creates an infinte loop
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input\n\n";
            continue;
        }
        cout << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string input_str = to_string(input);
        send(sock, input_str.c_str(), input_str.length()+1, 0); //sends the input to the server

        memset(buffer, 0, BUFFER_SIZE);

        if(input == 1 || input == 4) //sends the message needed to be searched
        {
            string message;
            cout << "Search: ";
            getline(cin, message); //gets what needs to be searched

            send(sock, message.c_str(), message.length(), 0); //sends to server

            read(sock, buffer, BUFFER_SIZE); //prints server response
            cout << buffer << endl;
            cout << endl;
        }
        else if(input == 2 || input == 3 || input == 6) //prints server response
        {
            memset(buffer, 0, BUFFER_SIZE);
            read(sock, buffer, BUFFER_SIZE);
            cout << buffer << endl;
            cout << endl;
        }
        else if(input == 5) //gets name, rating, and review for movies 
        {
            string movie;
            int rating;
            string review;
            string line2;

            cout << "Enter movie title: ";
            getline(cin, movie);
            cout << "Enter movie rating (1-5 whole numbers): ";
            cin >> rating;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter your review(2 sentences max): "; //keep it short
            getline(cin, review);

            send(sock, movie.c_str(), movie.length()+1, 0);

            usleep(10000); 
            
            string rating_str = to_string(rating);
            send(sock, rating_str.c_str(), rating_str.length()+1, 0);

            usleep(10000); 
            
            send(sock, review.c_str(), review.length(), 0);

            memset(buffer, 0, BUFFER_SIZE);
            read(sock, buffer, BUFFER_SIZE); //reads and prints response
            cout << buffer << endl << endl;
            
        }
        else //checks for valid input
        {
            cout << "Invalid input" << endl;
            cout << endl;
        }
    }

    close(sock); //closes socket
    return 0;
}