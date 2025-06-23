#include <cairo.h>
#include <cairo-pdf.h>

#define _USE_MATH_DEFINES
#include <cmath> // cos, sin, M_PI
#include <string> // string
#include <vector> // vector
#include <random> // srand
#include <algorithm> // random_shuffle
#include <ctime> // time

unsigned random_seed = std::time(nullptr);

struct draw_image_config_t
{
    int image_width = 0;
    int image_height = 0;
    double circle_radius = 0;
    int circle_sectors_number = 0;

    int circle_sector_line_width = 1;
    double circle_start_sector_angle = 0.0;
    double circle_end_sector_angle = 2 * M_PI;
    double text_rotation = M_PI / 2;
    int text_font_size = 32;
    std::string text_font_famaly = "Sans-Serif";

    std::string inner_image_file_path;

    double circle_center_x() const noexcept { return image_width / 2; }
    double circle_center_y() const noexcept { return image_height / 2; }

    double circle_sector_angle_distance() const { return circle_end_sector_angle / circle_sectors_number; }
    double circle_sector_mid_angle() const { return circle_sector_angle_distance() / 2; }
};

void draw_rectangle(draw_image_config_t const& draw_image_config, cairo_t* pointer)
{
    cairo_save(pointer);
    {
        cairo_set_source_rgb(pointer, 0, 0, 0);
        cairo_rectangle(pointer, 0, 0, draw_image_config.image_width, draw_image_config.image_height);
        cairo_fill(pointer);
    }
    cairo_restore(pointer);
}

void draw_cirle(draw_image_config_t const& draw_image_config, cairo_t* pointer)
{
    cairo_save(pointer);
    {
        cairo_set_source_rgb(pointer, 127 / 255.0, 127 / 255.0, 127 / 255.0);  // TODO: move setup circle color to config

        cairo_arc(pointer,
          draw_image_config.circle_center_x(), draw_image_config.circle_center_y(),
          draw_image_config.circle_radius,
          draw_image_config.circle_start_sector_angle, draw_image_config.circle_end_sector_angle
        );

        cairo_fill(pointer);
    }
    cairo_restore(pointer);
}

void draw_separation_lines(draw_image_config_t const& draw_image_config, cairo_t* pointer)
{
    cairo_save(pointer);
    {
        cairo_set_source_rgb(pointer, 64 / 255.0, 64 / 255.0, 64 / 255.0); // TODO: move setup seperation line color to config
        cairo_set_line_width(pointer, draw_image_config.circle_sector_line_width);
        for (double angle = draw_image_config.circle_start_sector_angle;
             angle < draw_image_config.circle_end_sector_angle;
             angle += draw_image_config.circle_sector_angle_distance())
        {
            double line_end_x = draw_image_config.circle_center_x() + cos(angle) * draw_image_config.circle_radius;
            double line_end_y = draw_image_config.circle_center_y() + sin(angle) * draw_image_config.circle_radius;
            cairo_move_to(pointer, draw_image_config.circle_center_x(), draw_image_config.circle_center_y());
            cairo_line_to(pointer, line_end_x, line_end_y);
        }
        cairo_stroke(pointer);
    }
    cairo_restore(pointer);
}

void draw_text(draw_image_config_t const& draw_image_config, cairo_t* pointer)
{
    cairo_save(pointer);
    {
        cairo_select_font_face(pointer, draw_image_config.text_font_famaly.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(pointer, draw_image_config.text_font_size);
        cairo_set_source_rgb(pointer, 255 / 255.0, 255 / 255.0, 255 / 255.0); // TODO: move setup font famaly, size and color to config

        int sector_index = 1;
        for (double angle = draw_image_config.circle_start_sector_angle;
             angle < draw_image_config.circle_end_sector_angle;
             angle += draw_image_config.circle_sector_angle_distance())
        {
            double mid_angle = angle + draw_image_config.circle_sector_mid_angle();
            double sector_center_x = draw_image_config.circle_center_x() + cos(mid_angle) * draw_image_config.circle_radius / 2;
            double sector_center_y = draw_image_config.circle_center_y() + sin(mid_angle) * draw_image_config.circle_radius / 2;

            cairo_save(pointer);
            {
                cairo_translate(pointer, sector_center_x, sector_center_y);
                cairo_move_to(pointer, 0, 0);
                cairo_rotate(pointer, mid_angle + draw_image_config.text_rotation);

                cairo_show_text(pointer, std::to_string(sector_index).c_str());
            }
            cairo_restore(pointer);
            sector_index += 1;
        }
    }
    cairo_restore(pointer);
}

void draw_image(draw_image_config_t const& draw_image_config, cairo_t* pointer)
{
    if (draw_image_config.inner_image_file_path.empty())
        return;

    cairo_save(pointer);
    {
        cairo_surface_t* image = cairo_image_surface_create_from_png(draw_image_config.inner_image_file_path.c_str());

        if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
            return;

        int image_width = cairo_image_surface_get_width(image);
        int image_height = cairo_image_surface_get_height(image);

        cairo_set_source_surface(pointer,
            image,
            draw_image_config.circle_center_x() - image_width / 2.0, draw_image_config.circle_center_y() - image_height / 2.0
        );
        cairo_paint(pointer);

        cairo_surface_destroy(image);
    }
    cairo_restore(pointer);
}

void draw_random_lines(draw_image_config_t const& draw_image_config, cairo_t* pointer)
{
    std::vector<double> sector_mid_angles;
    for (double angle = draw_image_config.circle_start_sector_angle;
         angle < draw_image_config.circle_end_sector_angle;
         angle += draw_image_config.circle_sector_angle_distance())
    {
        sector_mid_angles.push_back(angle + draw_image_config.circle_sector_mid_angle());
    }
    std::random_shuffle(sector_mid_angles.begin(), sector_mid_angles.end());

    // Draw first line
    cairo_save(pointer);
    {
        cairo_move_to(pointer, draw_image_config.circle_center_x(), draw_image_config.circle_center_y());
        double line_end_x = draw_image_config.circle_center_x() + std::cos(sector_mid_angles[0]) * draw_image_config.circle_radius;
        double line_end_y = draw_image_config.circle_center_y() + std::sin(sector_mid_angles[0]) * draw_image_config.circle_radius;
        cairo_line_to(pointer, line_end_x, line_end_y);
        cairo_set_source_rgb(pointer, 255 / 255.0, 0 / 255.0, 0 / 255.0);
        cairo_set_line_width(pointer, 2);
        cairo_stroke(pointer);
    }
    cairo_restore(pointer);

    // Draw second line
    cairo_save(pointer);
    {
        cairo_move_to(pointer, draw_image_config.circle_center_x(), draw_image_config.circle_center_y());

        double line_end_x = draw_image_config.circle_center_x() + std::cos(sector_mid_angles[1]) * draw_image_config.circle_radius;
        double line_end_y = draw_image_config.circle_center_y() + std::sin(sector_mid_angles[1]) * draw_image_config.circle_radius;
        cairo_line_to(pointer, line_end_x, line_end_y);

        cairo_set_source_rgb(pointer, 0 / 255.0, 0 / 255.0, 255 / 255.0);
        cairo_set_line_width(pointer, 4);

        std::vector<double> dashes = {10.0, 5.0};
        cairo_set_dash(pointer, dashes.data(), dashes.size(), 0);

        cairo_stroke(pointer);
    }
    cairo_restore(pointer);

    // Draw third line
    cairo_save(pointer);
    {
        cairo_move_to(pointer, draw_image_config.circle_center_x(), draw_image_config.circle_center_y());
        double line_end_x = draw_image_config.circle_center_x() + std::cos(sector_mid_angles[2]) * draw_image_config.circle_radius;
        double line_end_y = draw_image_config.circle_center_y() + std::sin(sector_mid_angles[2]) * draw_image_config.circle_radius;
        cairo_line_to(pointer, line_end_x, line_end_y);
        cairo_set_source_rgba(pointer, 255 / 255.0, 255 / 255.0, 0 / 255.0, 127 / 255.0);
        cairo_set_line_width(pointer, 1);
        cairo_stroke(pointer);
    }
    cairo_restore(pointer);
}

void draw_all(draw_image_config_t const& draw_image_config, cairo_t* pointer)
{
    // Circle geometry
    draw_rectangle(draw_image_config, pointer);  // Draw rectangle
    draw_cirle(draw_image_config, pointer); // Draw circle
    draw_separation_lines(draw_image_config, pointer); // Draw separation lines
    draw_text(draw_image_config, pointer); // Draw text with true rotation

    // Circle center
    draw_image(draw_image_config, pointer); // Draw inner image

    // Lines
    draw_random_lines(draw_image_config, pointer); // Draw random lines
}

void create_image_png(draw_image_config_t const& draw_image_config, std::string const& file_path)
{
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, draw_image_config.image_width, draw_image_config.image_height);
    cairo_t* pointer = cairo_create(surface);

    draw_all(draw_image_config, pointer);
    cairo_surface_write_to_png(surface, file_path.c_str());

    cairo_destroy(pointer);
    cairo_surface_destroy(surface);
}

void create_image_pdf(draw_image_config_t const& draw_image_config, std::string const& file_path)
{
    cairo_surface_t* surface = cairo_pdf_surface_create(file_path.c_str(), draw_image_config.image_width, draw_image_config.image_height);
    cairo_t* pointer = cairo_create(surface);

    draw_all(draw_image_config, pointer);
    cairo_show_page(pointer);

    cairo_destroy(pointer);
    cairo_surface_destroy(surface);
}

int main(int argc, char** argv)
{
    std::string const resources_dir = ASTROLOGY_PROJECT_RESOURCES_DIR;
    std::string const inner_image_file_path = resources_dir + "/Sprites/sun.png";

    draw_image_config_t draw_image_config;

    draw_image_config.image_width = 800;
    draw_image_config.image_height = 800;
    draw_image_config.circle_radius = 300;
    draw_image_config.circle_sectors_number = 12;
    draw_image_config.inner_image_file_path = inner_image_file_path;

    std::srand(random_seed); // commit seed
    create_image_png(draw_image_config, resources_dir + "/output.png");

    std::srand(random_seed); // commit seed
    create_image_pdf(draw_image_config, resources_dir + "/output.pdf");

    return 0;
}
