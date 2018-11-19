#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <cmath>
#include <stack>
#include <random>

using namespace std;
#define WIDTH 85
#define HEIGHT 45

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0, 1);

class GridRenderer
{
    char grid[WIDTH][HEIGHT]{};

public:
    void clearGrid()
    {
        memset(grid, 0, WIDTH * HEIGHT);
    }

    void drawCircleSection(char symbol, double angle, int width, int height, int xOffset=0, int yOffset=0)
    {
        int y = (int)(sin(angle) * height / 2) + height / 2;
        int x = (int)(cos(angle) * width / 2) + width / 2;
        grid[x + xOffset][y + yOffset] = symbol;
    }

    void drawCircle(char symbol, int width, int height, int xOffset=0, int yOffset=0)
    {
        for (double angle = 0; angle < M_PI * 2; angle += 0.1)
        {
            drawCircleSection(symbol, angle, width, height, xOffset, yOffset);
        }
    }

    void clearScreen()
    {
        //TODO is there really no better, multi-platform way of doing this...
        cout << string( 100, '\n' );
    }

    void renderGridToScreen()
    {
        for(int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                cout << (grid[x][y] == 0 ? ' ' : grid[x][y]);
            }
            cout << endl;
        }
        cout << endl << endl;
    }

};

struct FidgetAction
{
    double force;
    double delay;

    FidgetAction(double force, double delay): force(force), delay(delay) {}
};

stack<FidgetAction> fillActions(double currentSpeed, double targetSpeed)
{
    auto actions = stack<FidgetAction>();
    double speedDiff = abs(targetSpeed - currentSpeed);
    while (speedDiff > 0)
    {
        double actionForce = dis(gen) * 5;
        double actionDelay = dis(gen) * 2;
        if (currentSpeed <= targetSpeed)
        {
            actions.push(FidgetAction(actionForce, actionDelay));
        }
        else
        {
            actions.push(FidgetAction(-actionForce, actionDelay));
        }
        speedDiff -= actionForce;
    }

    return actions;
}

int main()
{
    GridRenderer gridRenderer;
    double angle = 0;
    double outerCircleRatio = 4;
    double innerCircleRatio = 8;
    double currentSpeed = 10;
    auto actions = fillActions(currentSpeed, 20);

    double frameDuration = 0.020;

    while (true)
    {
        if (actions.empty())
        {
            actions = fillActions(currentSpeed, dis(gen) * 10 - 5);
        }

        FidgetAction& nextAction = actions.top();
        nextAction.delay -= frameDuration;
        if (actions.top().delay <= 0) {
            currentSpeed += actions.top().force;
            actions.pop();
        }

        gridRenderer.clearGrid();

        cout << "                            <<<<<    fidget-cli    >>>>>            " << endl << endl << endl;
        outerCircleRatio = 4 + sin(angle) * 2.5;
        innerCircleRatio = 8 + sin(angle) * 2.5;
        gridRenderer.drawCircle('#', WIDTH / outerCircleRatio, HEIGHT / outerCircleRatio, WIDTH / 2 - (WIDTH / outerCircleRatio / 2), HEIGHT / 2 - (HEIGHT / outerCircleRatio / 2));
        gridRenderer.drawCircle('X', WIDTH / innerCircleRatio, HEIGHT / innerCircleRatio, WIDTH / 2 - (WIDTH / innerCircleRatio / 2), HEIGHT / 2 - (HEIGHT / innerCircleRatio / 2));
        gridRenderer.drawCircleSection('O', angle, WIDTH, HEIGHT);
        gridRenderer.drawCircleSection('O', angle + 2 * M_PI / 3, WIDTH, HEIGHT);
        gridRenderer.drawCircleSection('O', angle + 4 * M_PI / 3, WIDTH, HEIGHT);

        gridRenderer.clearScreen();
        gridRenderer.renderGridToScreen();
        cout << "Spinning at " << currentSpeed << " rads per sec" << endl;
        cout << "                            <<<<<    zoom zoom    >>>>>            " << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(20ms));
        angle += currentSpeed * frameDuration;

        currentSpeed *= 0.99;
    }

    return 0;
}