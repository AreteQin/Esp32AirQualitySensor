module esp32_case()
{
    // Dimensions (adjust as needed for your ESP32 model)
    sensor_length    = 62;
    sensor_width     = 49;  // mm
    sensor_offset    = 0.7;
    
    esp_board_width  = 51.5;
    esp_board_length = 28;
    
    screen_width = 29;
    screen_length = 19.5;
    screen_posX = esp_board_length + 5;
    
    wall_thickness   = 2;   // mm
    case_width       = esp_board_width + wall_thickness;       // mm
    case_length      = sensor_length + wall_thickness + 3;       // mm
    case_height      = 20;  // mm
    
    // Column size (adjust if needed)
    column_diameter  = 2.8; // mm
    column_height = 10;
    column_lcd_diameter = 1.9;

    // Flange dimensions
    flange_thickness = 1;   // thickness in Y
    flange_height    = 3; 
    flange_length = 20;

    union() {
        //
        // 1) Main case shell
        //
        difference() {
            // Outer block
            cube([case_length, case_width, wall_thickness + case_height]);
            
            // Hollow interior
            translate([wall_thickness, wall_thickness, wall_thickness])
                cube([
                    case_length - 2*wall_thickness,
                    case_width  - 2*wall_thickness,
                    case_height
                ]);

            // Window on the base
            translate([
                screen_posX + wall_thickness,
                (case_width - screen_width) / 2,
                0
            ])
            cube([screen_length, screen_width, wall_thickness + 0.1]);
            // Window on the wall
            translate([
                wall_thickness+7,
                case_width-wall_thickness,
                wall_thickness
            ])
            cube([13, wall_thickness, case_height]);
        }

        // 4 support columns (existing)
        xPositions = [
            wall_thickness + sensor_offset + 0.5*column_diameter,
            wall_thickness + esp_board_length - sensor_offset - 0.5*column_diameter
        ];

        yPositions = [
            case_width - wall_thickness - sensor_offset - 0.5*column_diameter,         // Front columns
            wall_thickness + sensor_offset + 0.5*column_diameter                        // Rear columns
        ];

        for (xPos = xPositions)
            for (yPos = yPositions)
                translate([xPos, yPos, wall_thickness])
                    cylinder(h = column_height, d = column_diameter);

        // 4 support columns around the window
        // Calculate the window's edges
        window_left   = screen_posX + wall_thickness;
        window_right  = window_left + screen_length;
        window_bottom = (case_width - screen_width) / 2;
        window_top    = window_bottom + screen_width;

        // Offset so columns sit just outside the window cutout
        window_x_positions = [
            window_left   - 0.5*column_lcd_diameter-0.6,
            window_right  + 0.5*column_lcd_diameter+0.6
        ];

        window_y_positions = [
            window_bottom + 0.5*column_diameter,
            window_top    - 0.5*column_diameter
        ];

        for (xPos = window_x_positions)
            for (yPos = window_y_positions)
                translate([xPos, yPos, wall_thickness])
                    cylinder(h = column_height, d = column_lcd_diameter);
    }
}

// Call the module
esp32_case();
