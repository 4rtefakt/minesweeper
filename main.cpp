#include <time.h>
#include <vector>

#include <SFML/Graphics.hpp>

#define FPS 60


sf::Color colors[] = {
    sf::Color::Black,
    sf::Color::Green,
    sf::Color::Blue,
    sf::Color::Magenta,
    sf::Color::Red,
    sf::Color::Red,
    sf::Color::Red,
    sf::Color::Red,
    sf::Color::Red,
};


// Structure to store data for each Cell of the Grid
struct cell {
    bool        hasBomb;
    bool        hasFlag;
    bool        isRevealed;
    int         nbNearbyBombs;
    sf::Sprite  sprite;
    sf::Text    text;
};

// SFML window
sf::RenderWindow win;

// Graphic ressources
sf::Texture tex_bomb;
sf::Texture tex_flag;
sf::Texture tex_cell;
sf::Font    font;

// Dimensions of the grid
int grid_width  = 30;
int grid_height = 16;

// Number of bombs
int nb_bombs    = 99;

// Store the game state
bool lost       = false;

// Matrix that will store the game's grid
std::vector<std::vector<struct cell>>grid(grid_height, std::vector<struct cell>(grid_width));

// Functions definition
bool check_victory();
void draw_grid();
void free_ressources();
void gen_grid();
void handle_lclick();
void handle_rclick();
void init_game();
void render_game();
void reveal_cell(int x, int y);
void place_bombs();

int main()
{
    // Define base values
    // TODO: make basic values(grid_height, grid_width, nb_bombs) configurable through arguments or config files

    init_game();
    gen_grid();
    place_bombs();

    // Game loop
    while (win.isOpen()) {

        // Event loop
        sf::Event event;
        while (win.pollEvent(event)) {

            // Event handling
            switch(event.type) {

                // Event when the window/game is closed
                case sf::Event::Closed:
                    win.close();
                    break;

                // Event when the player clicks
                case sf::Event::MouseButtonPressed:
                    if(event.mouseButton.button == sf::Mouse::Left)
                        handle_lclick();
                    else if(event.mouseButton.button ==sf::Mouse::Right)
                        handle_rclick();
                    break;
            }
        }
        render_game();
    }
    return 0;
}

// Check if the board is victorious
bool check_victory() {
    bool has_won = true;

    for(int i=0;i<grid_height;++i)
        for(int j=0;j<grid_width;++j)
            has_won = has_won && ( grid[i][j].isRevealed && !grid[i][j].hasBomb ) || ( grid[i][j].hasBomb && !grid[i][j].isRevealed );

    return has_won;
}

// Draw the game's grid on the screen
void draw_grid() {
    for(int i=0;i<grid_height;++i)
        for(int j=0;j<grid_width;++j) {
            if(!lost) {
                if( grid[i][j].isRevealed || grid[i][j].hasFlag )
                    win.draw(grid[i][j].sprite);
                if( grid[i][j].isRevealed && grid[i][j].nbNearbyBombs > 0 ) {
                    win.draw(grid[i][j].text);
                }
            } else {
                if( !grid[i][j].hasBomb && grid[i][j].nbNearbyBombs==0 )
                    continue;
                else if( grid[i][j].hasBomb )
                    win.draw(grid[i][j].sprite);
                else
                    win.draw(grid[i][j].text);
            }
        }
}

// Initialize the matrix of vector that will store the game's grid
void gen_grid() {
    for(int i=0;i<grid_height;++i) {
        for(int j=0;j<grid_width;++j) {
            grid[i][j].hasBomb = false;
            grid[i][j].isRevealed = false;
            grid[i][j].nbNearbyBombs = 0;

            grid[i][j].sprite = sf::Sprite(tex_cell);
            grid[i][j].sprite.setPosition(sf::Vector2f(j*25, i*25));
            grid[i][j].sprite.setScale(sf::Vector2f(0.1f,0.1f));

            grid[i][j].text = sf::Text();
            grid[i][j].text.setFont(font);
            grid[i][j].text.setPosition(sf::Vector2f(j*25-2, i*25-5));
            grid[i][j].text.setCharacterSize(25);
            grid[i][j].text.setFillColor(sf::Color::Black);
        }
    }
}

// Handle a left click from the player
void handle_lclick() {
    if(lost) return;
    int x = sf::Mouse::getPosition(win).x/25;
    int y = sf::Mouse::getPosition(win).y/25;

    reveal_cell(x,y);

}

// Handle a right click from the player
void handle_rclick() {
    if(lost) return;
    int x = sf::Mouse::getPosition(win).x/25;
    int y = sf::Mouse::getPosition(win).y/25;

    // Toggle the flag on the cell and adapt texture as needed
    grid[y][x].hasFlag = !grid[y][x].hasFlag;
    grid[y][x].sprite = ( grid[y][x].hasFlag ? sf::Sprite(tex_flag) : sf::Sprite(tex_cell));
    grid[y][x].sprite.setScale(sf::Vector2f(0.1f,0.1f));
    grid[y][x].sprite.setPosition(sf::Vector2f(x*25, y*25));

}

// Initialize the window and load the assets
void init_game() {
    // Create the window
    win.create(sf::VideoMode(grid_width*25, grid_height*25), "minesweeper by artefakt");

    // Set FPS limit
    win.setFramerateLimit(FPS);

    // Load the bomb texture
    if(!tex_bomb.loadFromFile("assets/bomb.png")) {
        printf("error: couldn't load texture 'assets/bomb.png'\n");
        exit(1);
    }

    // Load the cell texture
    if(!tex_cell.loadFromFile("assets/cell.png")) {
        printf("error: couldn't load texture 'assets/cell.png'\n");
        exit(1);
    }

    // Load the flag texture
    if(!tex_flag.loadFromFile("assets/flag.png")) {
        printf("error: couldn't load texture 'assets/flag.png'\n");
        exit(1);
    }

    // Load the font
    if(!font.loadFromFile("assets/font.ttf")) {
        printf("error: couldn't load font 'assets/font.ttf'\n");
        exit(1);
    }
}

// Handle the reaction to the user loosing the game
void loose_game() {
    printf("you lost\n");
    lost = true;
}

// Place bombs throughout the game's grid
void place_bombs() {
    // Seed our random generator
    srand(time(NULL));

    // Initializing variables
    char i, j, x=0;
    do {
        // Pulling pairs of random numbers until they design a free cell
        i = rand() % grid_height;
        j = rand() % grid_width;

        // If the cell is free
        if(!grid[i][j].hasBomb) {

            // Now the cell has a bomb
            grid[i][j].hasBomb = true;
            grid[i][j].sprite.setTexture(tex_bomb);

            // Tag the adjacent cells accordingly
            for(char x=i-1;x<i+2;++x)
                for(char y=j-1;y<j+2;++y)
                    if( !( x < 0 || y < 0 || x > grid_height-1 || y > grid_width-1 ) )
                        grid[x][y].nbNearbyBombs++;

            // Increment the number of placed bombs
            ++x;
        }
    // Continue until reaching the desired number of bombs
    } while ( x < nb_bombs && grid[i][j].hasBomb );

    for(i=0;i<grid_height;++i) {
        for(j=0;j<grid_width;++j) {
            grid[i][j].text.setString(std::to_string(grid[i][j].nbNearbyBombs));
            grid[i][j].text.setFillColor(colors[grid[i][j].nbNearbyBombs]);
        }
    }

}

// Render the game window
void render_game() {
    // Draw the window background
    win.clear(sf::Color::Black);

    // In case the game ended, background will be white
    if(lost) win.clear(sf::Color::White);

    // Draw all our drawable elements
    draw_grid();

    // Display and commit the drawings to the window
    win.display();
}

// Reveal a cell from the grid
void reveal_cell(int y, int x) {

    // TODO: Debug (swapped x and y ?)
    printf("revealing cell %d %d\n", x, y);
    if( grid[x][y].hasBomb) {
        loose_game();
        return;
    } else if (grid[x][y].nbNearbyBombs > 0) {
        grid[x][y].isRevealed = true;
        printf("revealed but bomb nearby\n");
        return;
    }

    std::vector<sf::Vector2i>   stack;
    sf::Vector2i                t;
    stack.push_back(sf::Vector2i(x, y));

    while(stack.size() > 0) {

        t = stack.back();
        stack.pop_back();

        printf("checking %d %d\n", t.x, t.y);

        if( grid[t.x][t.y].nbNearbyBombs > 0 ) {
            grid[t.x][t.y].isRevealed = true;
            continue;
        }

        for(char i=t.x-1;i<t.x+2;++i) {
            for(char j=t.y-1;j<t.y+2;++j) {
                if( !( i < 0 || j < 0 || i > grid_height-1 || j > grid_width-1 ) ) {
                    if( !grid[i][j].isRevealed) {
                        printf("%d %d\t\tlimits: %d %d\n", i, j, grid_height-1, grid_width-1);
                        grid[i][j].isRevealed = true;
                        stack.push_back(sf::Vector2i(i, j));
                    }
                }
            }
        }
    }
}

void win_game() {
    printf("you won\n");
    win.close();
    exit(0);
}
