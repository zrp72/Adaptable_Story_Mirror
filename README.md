Originally a project made for my Junior year capstone project, I've taken the original c++ code and begun the slow process of rewriting it in c. Do note that this is why there is a mix of c and c++ files, as well as the reason some c++ files use heavy c style coding. In the end, this is a test ground for me to learn more about coding, and ~98% of it is written solely by me.

The overall premise of the Adaptable Story Mirror is a system that parses interactive stories written in a proprietary language to a displayed window that players can interact with. While the original parser performed a buffer dump before iterating through std::strings, the new system is purposefully written to almost exclusively move forward through the file. To understand how to write a story, a demo file is provided in order to learn the language.

But there's more to interactive stories than just text. As like any proper game, there are graphics too. Although the implementation of this is still in the early stages, ASM parses the provided READ_DIRECTIONS document(Name can be overridden through argc[1]) for the asset folder, before loading all textures and fonts through SDL libraries.

One main thing I'm proud about in this project is glyphtext.cxx. Although it does use aspects of SDL_ttf, glyphtext is my own implementation of a glyph rasterization and rendering system. It allows the display of stylized text, with aspects such as size, font, color, italic, and bold.

Planned changes to this project are:
Full replacement of std::unordered_map and std::vector for custom implementations(ETA: ~December 1)

Collaborators:
Bugfixing: William Pursell\(wrp\)
Demo Story: Quinn Pursell\(Rotary_Productions\)
