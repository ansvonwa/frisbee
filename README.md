# About
This is the documentation and implementation of a frisbee with an LED stripe to play at night.



# Hardware
## Parts
- *Eurodisc 175g* (polyethylene), http://www.eurodisc.biz/wp-content/uploads/Summer_175g_600.jpg
- LiPo *Battery*, 950mAh, take the one from your old mobile
- *step-up converter* to 5V (my led stripe prefers 5V)
- *ESP8266* on a WeMos D1 mini, any other microcontroller will do the job as long as you do not plan to use the wifi features of the ESP
- *MPU6050* accelerometer and gyroscope sensor
- LED stripe, WS2812 LEDs, I have a stripe with 60LEDs/m and 49 fit into the fringe
- small switch (On/Off)
- small plug (to charge)
- LiPo charger or similar

## Assembling
- I tried some playstic glues, but hot glue worked best by far.
- Place glue on top of two consecutive LEDs and press them from the inside in the fringe of the disc. Repeat until all LEDs are attached
- Add some glue between the edge of the stripe an the disc.
- Solder the electoronics
- Glue the battery close to the center of the disc. Keep in mind that the disc deforms on impact and very high accelerations act on the disc at the same moment. Covering the four corners worked for me.
- Glue the MPU6050 close to the center, but not right in it (2-3 cm should be good), when the disc is spun with more then 5.5 rotations/s, the limit of the gyroscope is reached. Then, the centrifugal force can be measured by the accelerometer and used to calculate the rotational speed.
- Fix everything else.
- You may want to cover the electornics by a piece of cardboard of similar.


# Software
See `Arduino/frisbee/frisbee.ino` for the source code.


