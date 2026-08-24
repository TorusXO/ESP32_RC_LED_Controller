

This is a combination of Android app and esp32 program based on arduino code. 

The purpose of the project is to have more control over LEDs in RC cars. The setup made mainly for RC drift cars but can also work with any RC with similar setup.

You will need on the hardware side: ESP32, PCA9685 - led and servo controller(pwm), XL63070 - buck-boost converter for stable 5v output, MPU-6050 - accelerometer and gyro.

You will also need some wires and connectors to hookup your car's electronics basically making the Y cables between your channel1 and channel2.

The final setup will look like this: car's ESC(ch2) - [y cable] - car's receiver - controller. Car's gyro/servo(ch1) - [y cable] - car's receiver - controller.

The setup allows for more preciese controls of LED channels connected to the PCA9685. You can have up to 16 channels for a single board. The boards can be extended making +16 channels for each daisy-connected PCA9685. But the code only supports a single PCA board right now. 

The way it works: you connect your LEDs to the assembled controller's PCA9685, then assign each connected LED/servo a role in the app, like if it's a backlight LED, tail lights, front passive lights, turning to the left/right. 

Then after saving and uploading the setup from the app - your car has passive/active behaviour for the periferals it has.

When you use your break dim red lights on the back are brighter, when you press backward after the full stop you will get white lights on indicating reverse, turning lights are active by detecting turns of the wheel dictated by signals from receiver. 

The controller behaviour is similar to Tamiya hop-on options controller but much more rich in features. For example lights are lit and turn off instantly with other controllers, this one gives a slight fade so the lights look more natural and classy. 

You can select how much light you need in the app so waste less battery, or you can turn them off complitely. 

You can control servos from the setup. Right now it's setup for only a single purpose to have the servo work as opener for pop-up headlights, but you can use it basically for any other purpose. 

You can have the exhaust leds, 2 of them of different colors like blue and orange or red to imitate the exhaust flares, this works based on the throttle + accelerometer readings. 

In the app you can calibrate/setup the servo/gyro of the controller, you can calibrate how much angle you need from a servo. 

The app also has profiles in case if you have multiple car bodies and want to install the other controllers in them. 

You can also do a small diagnostics for assembly step in the app - it detects which boards are connected, if some board isnt showing up - check soldering/connections/wires, use multimeter to check that the connection is there. 

The pinout for soldering: 
On esp32: 
* D21 - SDA
* D22 - SCL
* D35 - Ch1 signal
* D25 - Ch2 signal

On PCA9685 you need to connect 5v output to the central piece of the board. Use the left side for soldering up the SDA, SCL and 3v3 pins, dont try to connect the MPU6050 with the right side of the PCA9685, even though they must share the i2c pins it didn't work for me, so I end up connecting SDA,SCL,3v3  directly from esp32
<img width="2424" height="1754" alt="IMG_7220" src="https://github.com/user-attachments/assets/1f5a5b55-bf97-489a-a84b-8381b42b8ef7" />
<img width="2836" height="2156" alt="IMG_7219" src="https://github.com/user-attachments/assets/0c64f88f-6676-4508-a5a1-7439c57c2e9a" />
Ill soon get the PCB I made for this setup so it will look less messy and more compact. Probably will post it on Patreon since i've put much effort to the whole project, so if you would want to support me you can do it there.

The app is build for android 13 specifically for Retroid pocket 5, so you might need to build/adapt the app for your device. What's good is that controls from the gamepad are there.

<img width="1280" height="720" alt="photo_5215592887817544381_y" src="https://github.com/user-attachments/assets/bc111109-8efd-4aec-a947-7daccd2a94e2" />
<img width="1280" height="720" alt="photo_5215592887817544383_y" src="https://github.com/user-attachments/assets/27581c69-ece0-47bf-a120-72fc1840656f" />
<img width="1280" height="720" alt="photo_5215592887817544382_y" src="https://github.com/user-attachments/assets/d892cc41-221c-4fc6-9bd7-56421f20f973" />
