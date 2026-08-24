This is a combination of Android app and esp32 program based on arduino code. 
The purpose of the project is to have more control over LEDs in RC cars. The setup made mainly for RC drift cars but can also work with any RC with similar setup.
You will need on the hardware side: ESP32, PCA9685 - led and servo controller(pwm), XL63070 - buck-boost converter for stable 5v output, MPU-6050 - accelerometer and gyro.
You will also need some wires and connectors to hookup your car's electronics basically making the Y cables between your channel1 and channel2.
The final setup will look like this: car's ESC(ch2) - [y cable] - car's receiver - controller. Car's gyro/servo(ch1) - [y cable] - car's receiver - controller.
The setup allows for more preciese controls of LED channels connected to the PCA9685. You can have up to 16 channels for a single board. The boards can be extended making +16 channels for each daisy-connected PC9685. But the code only supports a single PCA board right now. 
The way it works: you connect your LEDs to the assembled controller's PCA9685, then assign each connected LED/servo a role in the app, like if it's a backlight LED, tail lights, front passive lights, turning to the left/right. 
Then after saving and uploading the setup from the app - your car has passive/active behaviour for the periferals it has.
When you use your break dim red lights on the back are brighter, when you press backward after the full stop you will get white lights on indicating reverse, turning lights are active by detecting turns of the wheel dictated by signals from receiver. 
The controller behaviour is similar to Tamiya hop-on options controller but much more rich in features. For example lights are lit and turn off instantly with other controllers, this one gives a slight fade so the lights look more natural and classy. 
You can select how much light you need in the app so waste less battery, or you can turn them off complitely. 
You can control servos from the setup. Right now it's setup for only a single purpose to have the servo work as opener for pop-up headlights, but you can use it basically for any other purpose. 
