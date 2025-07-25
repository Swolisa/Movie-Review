# Movie Review Database System

A client-server application for managing and querying movie reviews

## Description
This project implements a networked movie review system with:
- TCP server (C++) handling database operations
- Command-line client (C++) for user interaction
- SQLite database storing movies and reviews

**Key Features:**
- Search for movies in catalog
- View top-rated movies (average rating)
- See most-reviewed movies
- Read existing reviews
- Submit new reviews (1-5 star ratings)
- Multi-client support via thraeding

## Installation
**Requirements**
- Linus environment
- g++ compiler
- SQLite3 development libraries

**Build and Run**
1. Clone the repository:
   ```bash
   git clone https://github.com/Swolisa/Movie-review.git
   cd Movie-Review
   ```
2. Compile both components
   ```bash
   g++ -std=c++17 server.cpp -o server -lsqlite3 -pthread
   g++ -std=c++17 main.cpp -o client
   ```
3. First run the server:
   ```bash
   ./server
   ```
4. Then run client(s) in seperate terminals
   ```bash
   ./client
   ```

## Usage
Client interface
```bash
===============================
Connected to Movie Review Server
===============================

(1) Search a movie
(2) Top rated movies
(3) Most reviewed movie
(4) Get reviews for a movie
(5) Submit a review
(6) Terminate
Select an option [1-6]:
```
Example Workflow
1. Search for a movie (option 1)
2. Check top-rated movies (option 2)
3. Submit a review (option 5)
   ```bash
   Enter movie title: The Matrix
   Enter movie rating (1-5 whole numbers): 5
   Enter your review(2 sentences max): Groundbreaking visual effects and philosophy.
   ```

## Database Schema
movies table:
- id: INTEGER PRIMARY KEY
- title: TEXT (case-insensitive)
  
reviews table:
- id: INTEGER PRIMARY KEY
- movie_title: INTEGER (1-5)
- review_text: TEXT

## Technical Details
- Port: 8080 (hardcoded)
- Buffer size: 1024 bytes
- Threading: Each client gets dedicated thread
- Input validation: Handles invalid numeric inputs
- SQL Injection Protection: Uses prepared statements

## License
MIT 2025 Swolisa
