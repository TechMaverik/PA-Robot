#include "display_wrapper.h"

// --- Hardware Configuration ---
const int sensorPin = A0;
const int buzzerPin = 3;   
const int threshold = 500; 
const long BAUD_RATE = 115200;

// --- Musical Frequencies (Hz) ---
#define NOTE_G4  392
#define NOTE_D4  294
#define NOTE_AS3 233 // B Flat (Mournful)
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_A2  110 // Low Angry Tone

// --- Eye State Struct ---
struct EyeState { 
    int height; 
    int width; 
    int x; 
    int y; 
};

const int REF_EYE_HEIGHT = 40;
const int REF_EYE_WIDTH = 40;
const int REF_SPACE_BETWEEN_EYE = 10;
const int REF_CORNER_RADIUS = 10;

EyeState left_eye, right_eye;
bool wasDry = false;

// --- Timing & Animation Variables ---
unsigned long dryStartTime = 0;
unsigned long happyStartTime = 0;
unsigned long lastBlinkTime = 0;
bool animationDone = false;
int blinkInterval = 3000; 

// --- Engine Functions ---

void draw_eyes() {
    g_draw_filled_round_rect(int(left_eye.x - left_eye.width / 2), int(left_eye.y - left_eye.height / 2), left_eye.width, left_eye.height, REF_CORNER_RADIUS, G_COLOR_WHITE);
    g_draw_filled_round_rect(int(right_eye.x - right_eye.width / 2), int(right_eye.y - right_eye.height / 2), right_eye.width, right_eye.height, REF_CORNER_RADIUS, G_COLOR_WHITE);
}

void reset_eyes(bool update = true) {
    left_eye.height = REF_EYE_HEIGHT; left_eye.width = REF_EYE_WIDTH;
    right_eye.height = REF_EYE_HEIGHT; right_eye.width = REF_EYE_WIDTH;
    left_eye.x = 64 - REF_EYE_WIDTH / 2 - REF_SPACE_BETWEEN_EYE / 2;
    left_eye.y = 32;
    right_eye.x = 64 + REF_EYE_WIDTH / 2 + REF_SPACE_BETWEEN_EYE / 2;
    right_eye.y = 32;
    if (update) { g_clear_display(); draw_eyes(); g_update_display(); }
}

// --- Masking Helpers ---

void apply_angry_mask() {
    g_draw_filled_triangle(left_eye.x - 21, left_eye.y - 21, left_eye.x + 21, left_eye.y - 21, left_eye.x + 21, left_eye.y, G_COLOR_BLACK);
    g_draw_filled_triangle(right_eye.x - 21, right_eye.y - 21, right_eye.x + 21, right_eye.y - 21, right_eye.x - 21, right_eye.y, G_COLOR_BLACK);
}

void draw_realistic_tear(int x, int y) {
    display.fillTriangle(x, y - 4, x - 2, y, x + 2, y, G_COLOR_WHITE);
    display.fillCircle(x, y + 1, 3, G_COLOR_WHITE);
}

// --- Melodies ---

void playCryingMelody() {
    // Mournful descending sequence
    tone(buzzerPin, NOTE_G4, 150); delay(200);
    tone(buzzerPin, NOTE_D4, 150); delay(200);
    tone(buzzerPin, NOTE_AS3, 300); delay(400);
    noTone(buzzerPin);
}

void playHappyMelody() {
    // Cheerful ascending sequence
    tone(buzzerPin, NOTE_C5, 80); delay(100);
    tone(buzzerPin, NOTE_E5, 80); delay(100);
    tone(buzzerPin, NOTE_G5, 150); delay(150);
    noTone(buzzerPin);
}

// --- Animation Routines ---

void fast_blink() {
    int speed = 20; 
    reset_eyes(false);
    for(int i = 0; i < 2; i++) {
        left_eye.height -= speed; right_eye.height -= speed;
        g_clear_display(); draw_eyes(); g_update_display();
    }
    for(int i = 0; i < 2; i++) {
        left_eye.height += speed; right_eye.height += speed;
        g_clear_display(); draw_eyes(); g_update_display();
    }
}

void look_around_animation() {
    int moveAmount = 15;
    for(int i = 0; i < 2; i++) {
        left_eye.x -= moveAmount; right_eye.x -= moveAmount;
        g_clear_display(); draw_eyes(); g_update_display();
        delay(100);
        left_eye.x += moveAmount * 2; right_eye.x += moveAmount * 2;
        g_clear_display(); draw_eyes(); g_update_display();
        delay(100);
        left_eye.x -= moveAmount; right_eye.x -= moveAmount;
    }
    reset_eyes(true);
}

// --- State Functions ---

void show_happy() {
    reset_eyes(false);
    g_clear_display();
    draw_eyes(); 
    g_update_display();
}

void show_crying() {
    reset_eyes(false);
    left_eye.height = 12; // 30% open
    right_eye.height = 12;
    g_clear_display();
    draw_eyes();
    draw_realistic_tear(left_eye.x - 8, left_eye.y + 12);
    draw_realistic_tear(right_eye.x + 8, right_eye.y + 12);
    display.setTextSize(1);
    display.setCursor(35, 55);
    display.print("NEED WATER");
    g_update_display();
}

void show_angry() {
    reset_eyes(false);
    g_clear_display();
    draw_eyes();
    apply_angry_mask();
    display.setTextSize(1);
    display.setCursor(35, 55);
    display.print("NEED WATER");
    g_update_display();
}

// --- Setup & Loop ---

void setup() {
    Serial.begin(BAUD_RATE);
    pinMode(buzzerPin, OUTPUT);
    g_init_display();
    reset_eyes(true);
}

void loop() {
    int sensorValue = analogRead(sensorPin);

    if (sensorValue < threshold) {
        if (wasDry) { 
            playHappyMelody();
            wasDry = false; 
            happyStartTime = millis(); 
            animationDone = false;
        }
        
        if (!animationDone && (millis() - happyStartTime > 2000)) {
            look_around_animation();
            animationDone = true; 
        } else {
            if (millis() - lastBlinkTime > blinkInterval) {
                fast_blink();
                lastBlinkTime = millis();
                blinkInterval = random(3000, 6000);
            } else {
                show_happy();
            }
        }
        noTone(buzzerPin);
    } else {
        if (!wasDry) { dryStartTime = millis(); wasDry = true; }
        long elapsed = (millis() - dryStartTime) % 5000;
        
        if (elapsed < 3000) {
            show_crying();
            playCryingMelody(); // Plays every ~1 second while crying
        } else {
            show_angry();
            // Aggressive angry pulse
            tone(buzzerPin, NOTE_A2, 100); delay(150); noTone(buzzerPin);
        }
    }
    delay(50);
}


