#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
using namespace std;

#define PORT 8080 //sets the port number for the server
#define BUFFER_SIZE 1024

void server_function(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int input = 0;

    string message = "\n===============================\nConnected to Movie Review Server\n===============================\n";
    send(client_socket, message.c_str(), message.length(), 0); //sends the connection message
    cout << "Connected to Client " << client_socket << endl;


    sqlite3* db; //opens the sql database for use
    if (sqlite3_open("movies.db", &db) != SQLITE_OK) {
        cerr << "Thread: Failed to open DB\n";
        close(client_socket);
        return;
    }

    while(input != 6) //keeps the loop running until the client is finished 
    {
        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, BUFFER_SIZE); //reads the input and sets it to the variable
        input = atoi(buffer);
        
        if(input == 1) //checks to see if the movie is in the database
        {
            memset(buffer, 0, BUFFER_SIZE);
            read(client_socket, buffer, BUFFER_SIZE);
            string movie(buffer);

            sqlite3_stmt* stmt;
            string sql = "SELECT title FROM movies WHERE title LIKE ? COLLATE NOCASE;"; //format for the code needed to search sql database

            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) { //prepare sql into code that sqlite can run and continues if it suceeded
                sqlite3_bind_text(stmt, 1, movie.c_str(), -1, SQLITE_TRANSIENT); //binds the movie string to the ? placeholder in sql code

                string result = "Search Results:\n";
                if(sqlite3_step(stmt) == SQLITE_ROW) { //checks to see if theres another row in the database and continues if there is
                    string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)); //returns the title from the database and assigns it as string to title variable
                    result += title + " is in the catalog\n";
                }
                else
                {
                    result += movie + " is not in the catalog\n";
                }

                send(client_socket, result.c_str(), result.length(), 0); //sends result to client
            }
            sqlite3_finalize(stmt); //cleans up and deletes the sql line created
        }
        else if(input == 2) //gets the top rated movies and finds the averages of the movies ratings to pick the top ones
        {
            sqlite3_stmt* stmt; //prepares sql statement
            string sql = R"(
                SELECT movie_title, AVG(rating) as avg_rating
                FROM reviews
                GROUP BY movie_title
                ORDER BY avg_rating DESC
                LIMIT 5;
            )";

            string result = "Top Rated Movies:\n";

            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)); //fetches the title
                    double avg_rating = sqlite3_column_double(stmt, 1); //gets the average rating from the database

                    ostringstream oss;
                    oss << fixed << setprecision(2) << avg_rating; //sets the precision to 2 decimals
                    string avg_str = oss.str();

                    result += title + " - " + avg_str + "\n";
                }
                sqlite3_finalize(stmt); //cleans and deletes the sql statement
            } else {
                result = "Failed to retrieve top-rated movies.\n";
            }

            send(client_socket, result.c_str(), result.length(), 0); //sends to client
        }
        else if(input == 3) //gets the most reviewed movies and shows how many reviews it has
        {
            sqlite3_stmt* stmt; //prepares sql statment
            string sql = R"(
                SELECT movie_title, COUNT(*) as review_count
                FROM reviews
                GROUP BY movie_title
                ORDER BY review_count DESC
                LIMIT 5;
            )";

            string result = "Most Reviewed Movies:\n";

            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)); //sets the title
                    int count = sqlite3_column_int(stmt, 1); //gets the number of reviews it has
                    result += title + " (" + to_string(count) + " reviews)\n";
                }
                sqlite3_finalize(stmt); //cleans up the sql code
            } else {
                result = "Failed to retrieve most-reviewed movies.\n";
            }

            send(client_socket, result.c_str(), result.length(), 0); //sends to client
        }
        else if(input == 4) //gets the reviews for a movie
        {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes = read(client_socket, buffer, BUFFER_SIZE);
            buffer[bytes] = '\0';
            string movie(buffer); //reads in the name of the movie and sets the movie in buffer to string

            sqlite3_stmt* stmt; //prepares sql statement
            string sql = "SELECT rating, review_text FROM reviews WHERE movie_title = ? COLLATE NOCASE;"; //checks databse for name and it is not case sensitive

            string result = "Reviews for \"" + movie + "\":\n";

            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, movie.c_str(), -1, SQLITE_TRANSIENT);

                bool found = false; //checks to seeif there are any movies with that name
                while (sqlite3_step(stmt) == SQLITE_ROW) { //loop keeps going if there is another row returned
                    found = true;
                    int rating = sqlite3_column_int(stmt, 0); //fetches the rating
                    string review = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)); //fetches the review
                    result += "Rating: " + to_string(rating) + " | Review: " + review + "\n"; //adds it to the string 
                }

                if (!found) {
                    result += "No reviews found for this movie.\n"; //if no movie in database it returns message
                }

                sqlite3_finalize(stmt);
            } else {
                result = "Failed to retrieve reviews.\n";
            }

            send(client_socket, result.c_str(), result.length(), 0);
        }
        else if(input == 5) //inputs movie into database
        {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(client_socket, buffer, BUFFER_SIZE);
            if (n <= 0) break;
            buffer[n] = '\0'; 
            string movie(buffer);
            
            memset(buffer, 0, BUFFER_SIZE);
            n = read(client_socket, buffer, BUFFER_SIZE);
            if (n <= 0) break;
            buffer[n] = '\0'; 
            int rating = atoi(buffer);
            
            memset(buffer, 0, BUFFER_SIZE);
            n = read(client_socket, buffer, BUFFER_SIZE);
            if (n <= 0) break;
            buffer[n] = '\0'; 
            string review(buffer);

            sqlite3_stmt* stmt; //prepares sql statement
            string findSql = "SELECT id FROM movies WHERE title = ? COLLATE NOCASE;";
            if (sqlite3_prepare_v2(db, findSql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, movie.c_str(), -1, SQLITE_TRANSIENT);
                if (sqlite3_step(stmt) != SQLITE_ROW) { //checks to see if the movies is already in database
                    sqlite3_stmt* insStmt; //prepares second sql statement for insertion
                    string insSql = "INSERT INTO movies (title) VALUES (?);";
                    if (sqlite3_prepare_v2(db, insSql.c_str(), -1, &insStmt, NULL) == SQLITE_OK) {
                        sqlite3_bind_text(insStmt, 1, movie.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_step(insStmt); //inserts the movie name into database
                        sqlite3_finalize(insStmt); //cleans up second statment
                    }
                }
                sqlite3_finalize(stmt); //cleans up first statement
            }

            string insert_sql = "INSERT INTO reviews (movie_title, rating, review_text) VALUES (?, ?, ?);"; //new sql statement enters review and rating into database
            if (sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, movie.c_str(), -1, SQLITE_TRANSIENT); //sets the names, rating, and review into placeholders for sql statement
                sqlite3_bind_int(stmt, 2, rating);
                sqlite3_bind_text(stmt, 3, review.c_str(), -1, SQLITE_TRANSIENT);

                if (sqlite3_step(stmt) == SQLITE_DONE) { //checks to see if review was add and send message
                    string response = "Review added successfully!";
                    send(client_socket, response.c_str(), response.length(), 0);
                } else { //if not added sends message
                    string response = "Failed to add review.";
                    send(client_socket, response.c_str(), response.length(), 0);
                }
            }
            sqlite3_finalize(stmt); //cleans up sql statement
        }
        else if(input == 6) //if termination input is detected it send message
        {
            strcpy(buffer, "Connection terminated");
            send(client_socket, buffer, strlen(buffer), 0);
        }
    }

    sqlite3_close(db); //closes the database and the connection to client
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    sqlite3* db; //opens database
    if (sqlite3_open("movies.db", &db) != SQLITE_OK) {
        cerr << "Failed to open DB: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    //creates movie names database if not already created
    const char* create_movies = R"(
        CREATE TABLE IF NOT EXISTS movies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT COLLATE NOCASE NOT NULL
        );
    )";

    //creates rating and review database if not already created
    const char* create_reviews = R"(
        CREATE TABLE IF NOT EXISTS reviews (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            movie_title TEXT COLLATE NOCASE NOT NULL,
            rating INTEGER CHECK(rating BETWEEN 1 AND 5),
            review_text TEXT
        );
    )";

    char* errMsg = nullptr; //prepares error message pointer
    if (sqlite3_exec(db, create_movies, NULL, NULL, &errMsg) != SQLITE_OK) { //executes the table creation statement
        cerr << "Error creating movies table: " << errMsg << endl; //prints out error if there is one
        sqlite3_free(errMsg); //frees the error message memory space
        sqlite3_close(db); //closes database
        return 1;
    }

    if (sqlite3_exec(db, create_reviews, NULL, NULL, &errMsg) != SQLITE_OK) { //repeated for second table
        cerr << "Error creating reviews table: " << errMsg << endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db); //closes database

    server_fd = socket(AF_INET, SOCK_STREAM, 0); //creates the listening socket
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); //allows quick reuse of port and address if server restarts

    address.sin_family = AF_INET; //builds the structure for the socket address
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (sockaddr*)&address, sizeof(address)); //binds the socket to a chosen port

    listen(server_fd, 3); //listens for client
    cout << "Server listening on port " << PORT << "...\n";

    while (1) {
        new_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen); //waits for connection and returns socket fd for a client
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        thread client_thread(server_function, new_socket); //creads a thread for the client
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}