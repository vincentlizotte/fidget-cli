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

//#define EMOJI_MODE // For the big thinkers. Probably not supported by a lot of terminals.

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0, 1);

class GridRenderer
{
    // Add an extra character on all sides, to account for overdrawing.
    // When drawing a point, +1 is then added to its coordinates to hide this overdrawing
    char grid[WIDTH + 2][HEIGHT + 2]{};

public:
    void clearGrid()
    {
        memset(grid, 0, (WIDTH + 2) * (HEIGHT + 2));
    }

    void drawPoint(char symbol, int x, int y, int lineWidth=1)
    {
        int adjustedX = x + 1;
        int adjustedY = y + 1;
        int adjustedLineWidth = lineWidth - 1;

        for (int lineWidthX = -adjustedLineWidth; lineWidthX <= adjustedLineWidth; lineWidthX++)
        {
            for (int lineWidthY = -adjustedLineWidth; lineWidthY <= adjustedLineWidth; lineWidthY++)
            {
                if (abs(lineWidthX) + abs(lineWidthY) <= adjustedLineWidth)
                {
                    grid[x + 1 + lineWidthX][y + 1 + lineWidthY] = symbol;
                }
            }
        }
    }

    void drawCircleSection(char symbol, double angle, int width, int height, int lineWidth=1, int xOffset=0, int yOffset=0)
    {
        int x = (int)(cos(angle) * width / 2) + width / 2;
        int y = (int)(sin(angle) * height / 2) + height / 2;
        drawPoint(symbol, x + xOffset, y + yOffset, lineWidth);
    }

    void drawCircle(char symbol, int width, int height, int xOffset=0, int yOffset=0)
    {
        for (double angle = 0; angle < M_PI * 2; angle += 0.1)
        {
            drawCircleSection(symbol, angle, width, height, 1, xOffset, yOffset);
        }
    }

    void drawArmEndpoint(double angle, int width, int height)
    {
        int x = (int)(cos(angle) * width / 2) + width / 2;
        int y = (int)(sin(angle) * height / 2) + height / 2;
        drawPoint('^', x, y-1);
        drawPoint('<', x-1, y);
        drawPoint('o', x, y);
        drawPoint('>', x+1, y);
        drawPoint('v', x, y+1);

    }

    void drawArm(char symbol, double angle, int width, int height, double outerCircleRatio)
    {
        int endX = (int)(cos(angle) * width / 2) + width / 2;
        int endY = (int)(sin(angle) * height / 2) + height / 2;

        drawLine(symbol, width / 2, height / 2, endX, endY, 1 / outerCircleRatio);
    }

    void drawLine(char symbol, int startX, int startY, int endX, int endY, double fractionToSkip)
    {
        pair<double, double> lineVector = make_pair(endX - startX, endY - startY);
        double norm = sqrt(lineVector.first * lineVector.first + lineVector.second * lineVector.second);
        pair<double, double> normalizedVector = make_pair(lineVector.first / norm, lineVector.second / norm);
        double lengthToSkip = norm * fractionToSkip;
        for (double progress = 0; progress < norm; progress += 0.2)
        {
            if (progress < lengthToSkip)
            {
                continue;
            }
            pair<double, double> currentPoint = make_pair(startX + progress * normalizedVector.first, startY + progress * normalizedVector.second);
            drawPoint(symbol, (int)currentPoint.first, (int)currentPoint.second);
        }

    }

    void clearScreen()
    {
        //TODO is there really no better, multi-platform way of doing this...
        cout << string( 100, '\n' );
    }

    void renderGridToScreen()
    {
        for(int y = 0; y < HEIGHT + 2; y++)
        {
            for (int x = 0; x < WIDTH + 2; x++)
            {

#ifdef EMOJI_MODE
                if (grid[x][y] == '^' || grid[x][y] == '<' || grid[x][y] == 'o' || grid[x][y] == '>' || grid[x][y] == 'v')
                {
                    cout << "🤔";
                }
                else
                {
                    cout << (grid[x][y] == 0 ? ' ' : grid[x][y]);
                }
#else
                cout << (grid[x][y] == 0 ? ' ' : grid[x][y]);
#endif

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
    double currentSpeed = 3;
    auto actions = fillActions(currentSpeed, 10);

    double frameDuration = 0.020;
    auto frameDurationMs = 20ms;

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

        outerCircleRatio = 4 + sin(angle) * 2;
        innerCircleRatio = 8 + sin(angle) * 2.5;

        // Draw the 2 inner expanding circles
        gridRenderer.drawCircle('#', WIDTH / outerCircleRatio, HEIGHT / outerCircleRatio, WIDTH / 2 - (WIDTH / outerCircleRatio / 2), HEIGHT / 2 - (HEIGHT / outerCircleRatio / 2));
        gridRenderer.drawCircle('X', WIDTH / innerCircleRatio, HEIGHT / innerCircleRatio, WIDTH / 2 - (WIDTH / innerCircleRatio / 2), HEIGHT / 2 - (HEIGHT / innerCircleRatio / 2));

        // Draw the arms
        gridRenderer.drawArm('.', angle, WIDTH, HEIGHT, outerCircleRatio);
        gridRenderer.drawArm('.', angle + 2 * M_PI / 3, WIDTH, HEIGHT, outerCircleRatio);
        gridRenderer.drawArm('.', angle + 4 * M_PI / 3, WIDTH, HEIGHT, outerCircleRatio);

        // Draw the ends of the arms
        gridRenderer.drawArmEndpoint(angle, WIDTH, HEIGHT);
        gridRenderer.drawArmEndpoint(angle + 2 * M_PI / 3, WIDTH, HEIGHT);
        gridRenderer.drawArmEndpoint(angle + 4 * M_PI / 3, WIDTH, HEIGHT);

        // Render to screen
        gridRenderer.clearScreen();
        cout << "                            <<<<<    fidget-cli    >>>>>            " << endl << endl << endl;
        gridRenderer.renderGridToScreen();
        cout << "Spinning at " << currentSpeed << " rads per sec" << endl;
        cout << "                            <<<<<    zoom zoom    >>>>>            " << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(frameDurationMs));
        angle += currentSpeed * frameDuration;

        currentSpeed *= 0.99;
    }

    return 0;
}