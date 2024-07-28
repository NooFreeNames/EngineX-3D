#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string>
#include <windows.h>
#include <ctime>
#include <vector>


#define GET_VAR(var) \
getline(level, line); \
var = stod(line.substr(line.find('=') + 2));
#define RADIANS(degrees)(M_PI / 180 * degrees)

#define HORIZONTAL GetSystemMetrics(false)
#define VERTICAL GetSystemMetrics(true)


using namespace std;
using namespace sf;

double get_var(ifstream &level) {
    string line;
    getline(level, line);
    return stod(line.substr(line.find('=') + 2));
}

struct Creature {
    double x;
    double y;

    double visibility_range;

    double viewing_angle;
    double view_dir_x;
    double view_dir_y;

    double acceleration;
    double walking_speed;
    double speed_at_the_moment;
    double running_speed;


};

struct Scene {
    unsigned int brightness;
    double sensitivity;
    double beam_step;
    double screen_width;
    double number_of_beams;
    unsigned int number_of_segments;
};

void animations(Scene &, Creature &);

void rendering(Creature &, RenderWindow &, Texture *&, double **&, Scene &);

void event_handling(Creature &, RenderWindow &, Event &, Scene &);

Color color_calculation(Scene, Creature, double, int, double *, Vector2 <double>);


int main(int argc, char *argv[]) {
    string programPath = argv[0], programName;
    programName = programPath.substr(programPath.find_last_of('\\') + 1, string::npos);
    programPath = programPath.erase(programPath.find_last_of('\\') + 1, string::npos);

    ifstream level(programPath + "Levels\\lvl-1-conv.txt", ios_base::in);
    if (!level.is_open()) {
        cout << "Error open lvl-1-conv.txt" << endl;
        system("pause");
    } else {
        auto *textures = new Texture[9];
        if (!textures[0].loadFromFile(programPath + R"(Textures\T0.jpg)") or
            !textures[1].loadFromFile(programPath + R"(Textures\T1.png)") or
            !textures[4].loadFromFile(programPath + R"(Textures\T4.png)") or
            !textures[6].loadFromFile(programPath + R"(Textures\T6.jpg)") or
            !textures[7].loadFromFile(programPath + R"(Textures\T7.jpg)") or
            !textures[8].loadFromFile(programPath + R"(Textures\T8.jpg)")) {
            cout << "Error open Textures";
            system("pause");
        } else {
            textures[0].setSmooth(true);
            textures[0].setRepeated(true);
            textures[1].setSmooth(true);
            textures[1].setRepeated(true);
            textures[4].setSmooth(true);
            textures[4].setRepeated(true);
            textures[7].setSmooth(true);
            textures[7].setRepeated(true);
            textures[8].setSmooth(true);
            textures[8].setRepeated(true);
            textures[6].setSmooth(true);
            textures[6].setRepeated(true);
        }

        Creature player = {};
        player.x = get_var(level);
        player.y = get_var(level);
        player.visibility_range = 500;
        player.view_dir_x = RADIANS(0);
        player.viewing_angle = RADIANS(90);
        player.acceleration = 0.05;
        player.walking_speed = 1.0 / 20;
        player.speed_at_the_moment = player.walking_speed;
        player.running_speed = player.walking_speed * 1.5;

        Scene main_scene = {};
        main_scene.brightness = 5;
        main_scene.number_of_beams = 1920;
        main_scene.sensitivity = 300;
        main_scene.screen_width = get_var(level);
        main_scene.beam_step = player.viewing_angle / main_scene.number_of_beams;
        main_scene.number_of_segments = int(get_var(level));

        Vector2<double> winning = {};
        winning.x = get_var(level);
        winning.y = get_var(level);


        string line;
        auto **segments = new double *[main_scene.number_of_segments];
        for (int segment = 0; segment < main_scene.number_of_segments; segment++) {
            segments[segment] = new double[5];
            getline(level, line);
            istringstream flow(line);
            string buffer;

            for (int point = 0; point < 5; point++) {
                flow >> buffer;
                segments[segment][point] = stod(buffer);
            }
        }

        level.close();

        RenderWindow window(VideoMode(HORIZONTAL, VERTICAL), programName, Style::Fullscreen);
        window.setMouseCursorVisible(false);

        while (window.isOpen()) {
            Event event = {};
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed or Keyboard::isKeyPressed(Keyboard::Escape)) {
                    window.close();
                }
            }

            window.clear();
            event_handling(player, window, event, main_scene);
            animations(main_scene, player);
            rendering(player, window, textures, segments, main_scene);
            window.display();
        }

        for (int segment = 0; segment < main_scene.number_of_segments; segment++) {
            delete[] segments[segment];
        }
        delete[] segments;
    }

    return 0;
}

void event_handling(Creature &player, RenderWindow &window, Event &event, Scene &main_scene) {

    if (Keyboard::isKeyPressed(Keyboard::W)) {
        player.x = player.speed_at_the_moment * sin(player.view_dir_x) + player.x;
        player.y = player.speed_at_the_moment * cos(player.view_dir_x) + player.y;
    } else if (Keyboard::isKeyPressed(Keyboard::S)) {
        player.x = -player.speed_at_the_moment / 2 * sin(player.view_dir_x) + player.x;
        player.y = -player.speed_at_the_moment / 2 * cos(player.view_dir_x) + player.y;
    }

    if (Keyboard::isKeyPressed(Keyboard::A)) {
        player.x = -player.speed_at_the_moment / 2 * cos(player.view_dir_x) + player.x;
        player.y = player.speed_at_the_moment / 2 * sin(player.view_dir_x) + player.y;
    } else if (Keyboard::isKeyPressed(Keyboard::D)) {
        player.x = player.speed_at_the_moment / 2 * cos(player.view_dir_x) + player.x;
        player.y = -player.speed_at_the_moment / 2 * sin(player.view_dir_x) + player.y;
    } else if (Keyboard::isKeyPressed(Keyboard::R)) {
        player.x = 10;
        player.y = 10;
    }

    if (Keyboard::isKeyPressed(Keyboard::LShift)) {
        if (player.speed_at_the_moment <= player.running_speed) {
            player.speed_at_the_moment += player.acceleration;
        }
    } else {
        if (player.speed_at_the_moment > player.walking_speed) {
            player.speed_at_the_moment -= player.acceleration * 2;
        } else {
            player.speed_at_the_moment = player.walking_speed;
        }
    }


    if (event.type == Event::MouseMoved) {
        if (event.mouseMove.x < HORIZONTAL / 2.0) {
            player.view_dir_x = fmod(
                    player.view_dir_x - ((-event.mouseMove.x + double(HORIZONTAL) / 2) / main_scene.sensitivity),
                    M_PI * 2);

        } else if (event.mouseMove.x > HORIZONTAL / 2.0) {
            player.view_dir_x = fmod(
                    player.view_dir_x + ((event.mouseMove.x - double(HORIZONTAL) / 2) / main_scene.sensitivity),
                    M_PI * 2);
        }
        player.view_dir_y = (event.mouseMove.y - VERTICAL / 2.0);

        Mouse::setPosition(Vector2i(HORIZONTAL / 2, event.mouseMove.y), window);
    }
}


void rendering(Creature &player, RenderWindow &window, Texture *&textures, double **&segments, Scene &main_scene) {
    Vector2u size = window.getSize();
    const unsigned int WIN_WIDTH = size.x;
    const unsigned int WIN_HEIGHT = size.y;
    int max_brightness = 255;


//    const int sky_color = max_brightness * main_scene.brightness;
//    Sprite sky(textures[6],
//               IntRect(player.view_dir_x * (textures[6].getSize().x / (2 * M_PI)), 0, WIN_WIDTH, WIN_HEIGHT));
//    sky.move(0, -float(player.view_dir_y) - WIN_HEIGHT / 2);
//    sky.setColor(Color(sky_color, sky_color, sky_color));
//    window.draw(sky);

    const int brightness_of_earth = max_brightness / 6 * main_scene.brightness;
    float length_earth_segment = float(WIN_HEIGHT) / brightness_of_earth;
    int color_of_earth;
    for (int earth_segment = 0; earth_segment < brightness_of_earth; ++earth_segment) {
        RectangleShape earth(Vector2f(float(WIN_WIDTH), length_earth_segment));
        earth.move(0, float(WIN_HEIGHT) * 1.5 - earth_segment * length_earth_segment - float(player.view_dir_y));
        color_of_earth = -earth_segment + brightness_of_earth;
        earth.setFillColor(Color(color_of_earth / 2, color_of_earth / 2, color_of_earth / 3));
        window.draw(earth);
    }


    const double INITIAL_COLUMN_WIDTH = WIN_WIDTH / main_scene.number_of_beams;
    double column_height, column_width, total_columns_width = 0;

    double beam_angle = player.view_dir_x - player.viewing_angle / 2.0;

    double nearest_point, vector_z1, vector_z2, vector_z3, vector_z4;
    double distance;
    unsigned texture_num, segment_num;

    Vector2<double> vector_beam, intersection_point, nearest_intersection_point;


    double sum = 0, beam_angle_copy = beam_angle;
    for (int beam_number = 0; beam_number < main_scene.number_of_beams; ++beam_number) {
        sum += INITIAL_COLUMN_WIDTH / pow(cos(player.view_dir_x - beam_angle_copy), 2);
        beam_angle_copy += main_scene.beam_step;
    }
    double compression_ratio = sum / WIN_WIDTH;

    for (int beam_number = 0; beam_number < main_scene.number_of_beams; ++beam_number) {

        column_width = INITIAL_COLUMN_WIDTH / pow(cos(player.view_dir_x - beam_angle), 2) / compression_ratio;

        vector_beam.x = sin(beam_angle) * player.visibility_range;
        vector_beam.y = cos(beam_angle) * player.visibility_range;
        nearest_point = player.visibility_range;

        for (int segment = 0; segment < main_scene.number_of_segments; ++segment) {

            vector_z1 = vector_beam.x * (segments[segment][1] - player.y) -
                        (segments[segment][0] - player.x) * vector_beam.y;
            vector_z2 = vector_beam.x * (segments[segment][3] - player.y) -
                        (segments[segment][2] - player.x) * vector_beam.y;

            if (vector_z1 * vector_z2 < 0) {

                vector_z3 = (segments[segment][2] - segments[segment][0]) *
                            (vector_beam.y + player.y - segments[segment][1]) -
                            (vector_beam.x + player.x - segments[segment][0]) *
                            (segments[segment][3] - segments[segment][1]);

                vector_z4 = (segments[segment][2] - segments[segment][0]) * (player.y - segments[segment][1]) -
                            (player.x - segments[segment][0]) * (segments[segment][3] - segments[segment][1]);

                if (vector_z3 * vector_z4 < 0) {
                    intersection_point.x = segments[segment][0] + (segments[segment][2] - segments[segment][0]) *
                                                                  abs(vector_z1) / abs(vector_z2 - vector_z1) -
                                           player.x;
                    intersection_point.y = segments[segment][1] + (segments[segment][3] - segments[segment][1]) *
                                                                  abs(vector_z1) / abs(vector_z2 - vector_z1) -
                                           player.y;

                    distance = sqrt(pow(intersection_point.x, 2) + pow(intersection_point.y, 2)) *
                               cos(player.view_dir_x - beam_angle);

                    if (distance < nearest_point) {
                        nearest_point = distance;
                        nearest_intersection_point.x = intersection_point.x + player.x;
                        nearest_intersection_point.y = intersection_point.y + player.y;
                        segment_num = segment;
                        texture_num = int(segments[segment][4]);
                    }
                }
            }
        }

        if (nearest_point != player.visibility_range) {
            column_height = float(main_scene.screen_width / nearest_point * (WIN_HEIGHT * 3));

            double a;
            if (texture_num == 4) {
                a = 2;
            } else if ( texture_num == 0) {
                a = 1.45;
            } else {
                a = 4;
            }

            Sprite sprite(textures[texture_num], IntRect(ceil(
                    sqrt(pow(segments[segment_num][0] - nearest_intersection_point.x, 2) +
                         pow(segments[segment_num][1] - nearest_intersection_point.y, 2)) *
                    textures[texture_num].getSize().x / a), 0, ceil(column_width),
                                                         ceil(textures[texture_num].getSize().y)));
            sprite.setColor(color_calculation(main_scene, player, nearest_point, max_brightness, segments[segment_num],
                                              vector_beam));
            sprite.scale(column_width / ceil(column_width),
                         float(column_height / float(textures[texture_num].getSize().y)));
            sprite.move(total_columns_width, (float(WIN_HEIGHT) - column_height) / 2 - float(player.view_dir_y));
            window.draw(sprite);


        }

        total_columns_width += column_width;
        beam_angle += main_scene.beam_step;
    }
}

Color color_calculation(Scene main_scene, Creature player, double nearest_point, int max_brightness, double *segment,
                        Vector2 <double> vector_beam) {

    Vector2 <double> vector_wall = {segment[2] - segment[0], segment[3] - segment[1]};
    double cos_V = (vector_wall.x * vector_beam.x + vector_wall.y * vector_beam.y) / (sqrt(pow(vector_wall.x, 2) + pow(vector_wall.y, 2)) * sqrt(pow(vector_beam.x, 2) + pow(vector_beam.y, 2)));

    nearest_point /= main_scene.brightness;

    nearest_point = int(((max_brightness - max_brightness * nearest_point / player.visibility_range) / nearest_point)) / 1.5;
    int color;
    if (segment[4] == 8) {
        color = int(-((fabs((cos_V / 2.5))) * nearest_point) + nearest_point);
    } else {
        color = int(-((fabs((cos_V / 1.5))) * nearest_point) + nearest_point);
    }





    if (color <= 0) {
        return Color(0, 0, 0);
    } else if (color > max_brightness) {
        return Color(max_brightness, max_brightness, max_brightness);
    } else {
        return Color(color, color, color);
    }
}

void animations(Scene &main_scene, Creature &player) {
//    player.viewing_angle -= 0.001;
//    player.viewing_angle = fmod(player.viewing_angle, M_PI);
//
//    main_scene.beam_step = player.viewing_angle / main_scene.number_of_beams;
}