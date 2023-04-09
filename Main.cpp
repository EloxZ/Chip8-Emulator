#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "Chip8.h"

void drawVideo(sf::RenderWindow& window, Chip8& chip, unsigned int videoScale);
int keyCodeIndex(sf::Keyboard::Key keyCode);

int main()
{
    Chip8 chip;
    const unsigned int videoScale = 15;
    const float sample_rate = 44100;
    const float frequency = 880; // 880 Hz = A5
    const float duration = 0.1; // 0.1 seconds
    const float amplitude = 30000;
    std::vector<sf::Int16> beep;
    
    for (float t = 0; t < duration; t += 1 / sample_rate) {
        float y = 0;
        y += sin(2 * M_PI * frequency * t);
        y += sin(2 * M_PI * frequency * 2 * t) / 2;
        y += sin(2 * M_PI * frequency * 4 * t) / 4;
        y += sin(2 * M_PI * frequency * 8 * t) / 8;
        y += sin(2 * M_PI * frequency * 16 * t) / 16;
        y /= 2; // normalize the amplitude

        short sample = y * amplitude;
        beep.push_back(sample);
    }

    sf::RenderWindow window(sf::VideoMode(DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale), "Chip-8 Emulator");
    window.setFramerateLimit(60);

    sf::SoundBuffer buffer;
    buffer.loadFromSamples(beep.data(), beep.size(), 1, sample_rate);

    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.setVolume(50);

    chip.loadROM("pong.ch8");
    std::thread cpuThread([&chip]() {
        chip.startCycle(1.43);
    });

    sf::Event event;
    bool isEvent = false;
    bool isBeeping = false;
    sf::Clock beepClock;
    
    while (window.isOpen())
    {
        if (isBeeping) {
            if (beepClock.getElapsedTime().asMilliseconds() >= 100) {
                isBeeping = false;
            }
        } else if (chip.soundTimer > 0) {
            beepClock.restart();
            sound.play();
            isBeeping = true;
            chip.soundTimer = 0;
        }

        if (chip.drawFlag) {
            chip.drawFlag = false;
            drawVideo(window, chip, videoScale);
        }

        while (window.pollEvent(event)) {   
            if (event.type == sf::Event::Closed) {
                std::cout << "Closing" << std::endl;
                chip.halt = true;
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                int index = keyCodeIndex(event.key.code);
                std::cout << "Key pressed " << index << std::endl;
                if (index != -1) {
                    chip.key[index] = 1;
                }
            } else if (event.type == sf::Event::KeyReleased) {
                int index = keyCodeIndex(event.key.code);
                std::cout << "Key released " << index << std::endl;
                if (index != -1) {
                    chip.key[index] = 0;
                }
            } else if (event.type == sf::Event::Resized) {
                //std::cout << "Refresh Video" << '\n';
                drawVideo(window, chip, videoScale);
            }
        }
    }

    return 0;
}

int keyCodeIndex(sf::Keyboard::Key keyCode) {
    switch (keyCode)
    {
        case sf::Keyboard::Num1:
            return 1;
        case sf::Keyboard::Num2:
            return 2;
        case sf::Keyboard::Num3:
            return 3;
        case sf::Keyboard::Num4:
            return 12;
        case sf::Keyboard::Q:
            return 4;
        case sf::Keyboard::W:
            return 5;
        case sf::Keyboard::E:
            return 6;
        case sf::Keyboard::R:
            return 13;
        case sf::Keyboard::A:
            return 7;
        case sf::Keyboard::S:
            return 8;
        case sf::Keyboard::D:
            return 9;
        case sf::Keyboard::F:
            return 14;
        case sf::Keyboard::Z:
            return 10;
        case sf::Keyboard::X:
            return 0;
        case sf::Keyboard::C:
            return 11;
        case sf::Keyboard::V:
            return 15;
        default:
            return -1; // return -1 for unrecognized key codes
    }
}

void drawVideo(sf::RenderWindow& window, Chip8& chip, unsigned int videoScale) {
    window.clear();

    for (unsigned int x=0; x < DISPLAY_WIDTH; x++)
    {
        for (unsigned int y=0; y < DISPLAY_HEIGHT; y++)
        {   
            if (chip.video[x][y])
            {
                sf::RectangleShape rectangle;
                rectangle.setSize(sf::Vector2f(videoScale, videoScale));
                rectangle.setFillColor(sf::Color::White);
                rectangle.setPosition(x * videoScale, y * videoScale);
                window.draw(rectangle);
            }
        }
    }

    window.display();
}
