LiDAR GTA V
============================
*A plugin for Grand Theft Auto V that generates a labeled LiDAR point cloud from the game environment.*

<img src="/samples/LiDAR Sample - Traffic.png">

## Requirements

* [Grand Theft Auto V](https://www.rockstargames.com/games/V)
* [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/)

## Installation

1. Install [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/) following its accompanying instructions
2. Copy-paste `LiDAR GTA V.asi` and the `LiDAR GTA V` folder from the latest [***release***](https://github.com/UsmanJafri/LiDAR-GTA-V/releases) into your GTA-V directory (the folder containing `GTAV.exe`)

## How to use

1. Navigate to `#your_game_directory#/LiDAR GTA V/LIDAR GTA V.cfg` and edit the parameters as needed. The parameters include: Horizontal FOV, Vertical FOV, Range, Density and Output filename
2. Start up GTA-V in story mode
3. Press `V` to cycle through camera modes until first person mode is enabled
4. Press `F6` to generate a LiDAR point cloud of the world.
5. Output file is located under `#game_directory#/LiDAR GTA V/#your_chosen_filename#.ply`
6. The output is a Polygon File Format (.PLY) file containing only vertex data
7. Vertex Color Code:

    | Vertex Color | Label |
    |---|---|
    | ![](https://via.placeholder.com/15/ff0000/000000?text=+) Red  | Vehicle |
    | ![](https://via.placeholder.com/15/00ff00/000000?text=+) Green | Humans and Animals |
    | ![](https://via.placeholder.com/15/0000ff/000000?text=+) Blue | Game props |
    | ![](https://via.placeholder.com/15/ffffff/000000?text=+) White | Road, Buildings and other hitable textures |
    | ![](https://via.placeholder.com/15/000000/000000?text=+) Black | No hit |

8. It is recommended to use [CloudCompare](https://github.com/cloudcompare/cloudcompare) (an open-source point-cloud viewer) to view the PLY file generated by *LiDAR GTA V*

## Developer Notes

* You will need Visual Studio 2019 or higher to open this project
* The project requires the ***Desktop development with C++*** workload to be installed in Visual Studio
* The project is targetted at Visual Studio Platform Toolset **v142**
* To make development easier, the project will automatically copy the compiled **LiDAR GTA V** plugin to the directory `D:\Games\GTAV\` after building
    
    You may want to set this path to point to your GTA-V installation directory. This can be done by right-clicking *LiDAR GTA V* in the *Solution Explorer* and editing the `Command Line` field under `Configuration Properties -> Build Events -> Post-Build Event -> Command Line`

## Contributing

* Any contributions to the project are welcomed, it is recommended to use GitHub [Pull requests](https://help.github.com/articles/using-pull-requests/)

## Acknowledgements

* [Native Trainer](http://www.dev-c.com/gtav/scripthookv/) (a ScriptHookV sample) was used as a starting point to import configuration

## License

* You are free to use this project in your work. However acknowledgement and a link to this repository would be greatly appreciated. If you do use this project in your work, I would love to hear about it, please drop me an email at `19100135@lums.edu.pk`
* The LiDAR-GTA-V source code is licensed under the [MIT license](LICENSE). Source content taken from other projects is tagged with the respective licenses wherever found

## Output Samples

<img src="/samples/LiDAR Sample - Michael Home.png">

<img src="/samples/LiDAR Sample - Test Area.png">

<img src="/samples/LiDAR Sample - Traffic.png">
