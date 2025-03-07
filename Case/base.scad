module esp32_case()
{
    // Dimensions (adjust as needed for your ESP32 model)
    sensor_length    = 62;
    sensor_width     = 49;  // mm
    sensor_offset    = 0.7;
    esp_board_width  = 52;
    esp_board_length = 30;
    wall_thickness   = 2;   // mm
    case_width       = esp_board_width + wall_thickness;       // mm
    case_length      = sensor_length + wall_thickness + 3;       // mm
    case_height      = 20;  // mm
    
    // Column size (adjust if needed)
    column_diameter  = 2.8; // mm
    column_height = 15;

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
                17 + wall_thickness,
                (case_width - sensor_width) / 2,
                0
            ])
            cube([40, sensor_width, wall_thickness + 0.1]);
        }

// 4 support columns
xPositions = [
    wall_thickness + sensor_offset + 0.5*column_diameter,
    case_length - wall_thickness - sensor_offset - 0.5*column_diameter
];

yPositions = [
    case_width - wall_thickness - sensor_offset - 0.5*column_diameter,         // Front columns
    case_width - wall_thickness + sensor_offset + 0.5*column_diameter - sensor_width // Rear columns
];

for (xPos = xPositions)
    for (yPos = yPositions)
        translate([xPos, yPos, wall_thickness])
            cylinder(h = column_height, d = column_diameter);

        
            // 3) Short vertical flanges on the inside of the front & back walls
        //
        // Front flange
        translate([
            wall_thickness + esp_board_length,
            wall_thickness,
            wall_thickness + case_height
        ])
        cube([
            flange_length,                // length of flange in X
            flange_thickness,  // thickness in Y
            flange_height      // extends downward in Z
        ]);

        // Back flange
        translate([
            wall_thickness + esp_board_length,
            case_width - wall_thickness - flange_thickness,
            wall_thickness + case_height
        ])
        cube([
            flange_length,
            flange_thickness,
            flange_height
        ]);
        
       // connecter
       translate([
            wall_thickness + esp_board_length,
            case_width - wall_thickness,
            case_height-1
        ])
           rotate([0, -90, 180]) {
        linear_extrude(height = 20) {
            polygon(points = [
                [0, 0],
                [3, 0],
                [3, flange_thickness]
            ]);
            
        }
    }
           translate([
            wall_thickness + esp_board_length+ flange_length,
             wall_thickness,
            case_height-1
        ])
           rotate([0, -90, 0]) {
        linear_extrude(height = 20) {
            polygon(points = [
                [0, 0],
                [3, 0],
                [3, flange_thickness]
            ]);
            
        }
    }
    }
}

// Call the module
esp32_case();
