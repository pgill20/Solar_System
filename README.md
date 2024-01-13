# Solar System

A solar system created using OpenGL.

A video showing off this project can be seen here: https://media.oregonstate.edu/edit/1_0h5978da

Users will need a glm folder in the root directory, which can be downloaded here: https://web.engr.oregonstate.edu/~mjb/cs550/ [1].

The working program is provided in the file titled “sample.cpp”. The program was compiled using the command “make” and then run using the command “./sample”.

A solar system was designedm to demonstrate texturing, distortion, lighting and different viewing options from Earth and Moon. In order to get all the planets to fit on the view, skewing and a lot of research had to be done. Outside resources were used to determine things like a planet’s radius, its orbital distance from the Sun etc. [2] [3]. All the planets (aside from the Sun) were modelled with respect to the Earth’s parameters. The calculations for the OrbitAngle parameter (defined within the Matrix for each planet) are shown in Table 1.1.

<div align="center">
  
| Days | Factor to multiply OrbitAngle |
| :---: | :---: |
| 88	| 4.147727273 |
| 225	| 1.622222222 |
| 365	| 1 |
| 687	| 0.531295488 |
| 4,333	| 0.084237249 |
| 10,759	| 0.033925086 |
| 30,687	| 0.011894287 |
| 60,190	| 0.00606413 |

*Table 1.1 OrbitAngle Factors for scaling*

</div>

The keyboard keys used for demonstration are as follows:

<div align="center">
  
| Kayboard Key | Outcome | 
| :---: | :---: |
| 2 | Turns on pointlight from the Sun after it has been Textured into the display |
| e, E	| EARTHVIEW |
| m, M	| MOONVIEW |
| k, K	| OUT, an outside view of the entire solar system |
| l, L	| OUTSIDEVIEW |

*Table 1.2 Keyboard input vs. Visual Output*

</div>

All the textures were taken from an outside resource [4]: https://www.solarsystemscope.com/textures/ 

Please use the “HIGH RESOLUTION 2k” Download for each planet for consistent results that match the demonstration video for this project. The textures were converted from JPEG to BMP format using the following resource [5]: https://cloudconvert.com/jpeg-to-bmp

A few screenshots of the program demonstrating the project requirements are also shown on the next pages in Figures 1.1, 1.2, 1.3 and 1.4.

<div align="center">
<img width="1000" alt="11" src="https://github.com/pgill20/Solar_System/assets/72182630/6db0546a-0507-4e0e-85e7-5f2e6b11f357">

*Figure 1.1 The entire solar system when the keyboard key ‘k’ or ‘K’ is pressed*
</div>

<div align="center">
<img width="1000" alt="12" src="https://github.com/pgill20/Solar_System/assets/72182630/62f71168-83d0-401e-a82b-d8bd5eb72520">

*Figure 1.2 A little more inside view of the solar system showcasing the lighting feature (as can be seen visibly on Earth)*
</div>

<div align="center">
<img width="1000" alt="13" src="https://github.com/pgill20/Solar_System/assets/72182630/a21eef53-61e4-46e8-a19b-b19675138e73">

*Figure 1.3 View from the Moon using the keyboard key ‘m’ or ‘M’*
</div>

<div align="center">
<img width="1000" alt="14" src="https://github.com/pgill20/Solar_System/assets/72182630/cdd1a72d-8121-42c8-9cda-ada4a072beb2">

*Figure 1.4 View from the Earth using the keyboard key ‘e’ or ‘E’*
</div>

**References:**

[1]	Bailey, M. (n.d.). CS 450/550 -- Fall Quarter 2022. Retrieved December 7, 2022, from https://web.engr.oregonstate.edu/~mjb/cs550/

[2]	https://nssdc.gsfc.nasa.gov/planetary/factsheet/

[3]	https://sos.noaa.gov/catalog/datasets/planet-rotations/#:~:text=Earth%3A%2023h%2056m%2C%201574%20km,10h%2033m%2C%2036%2C840%20km%2Fh

[4]	https://www.solarsystemscope.com/textures/

[5]	https://cloudconvert.com/jpeg-to-bmp

