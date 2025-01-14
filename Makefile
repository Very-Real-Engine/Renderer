NAME :=	Engine
SRC :=	$(wildcard src/*.cpp)
HDR :=	$(wildcard src/*.h)

all: $(NAME)_debug

release: $(NAME)_release

run : $(NAME)_debug
	@./$(NAME)_debug.exe

$(NAME)_debug: $(SRC) $(HDR)
	@cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug .
	@cmake --build build --config Debug
	@mv ./build/Debug/$(NAME).exe ./$(NAME)_debug.exe
	@echo [SUCCESS] $@ compiled successfully with debug mode and validation layers!

$(NAME)_release:
	@cmake -Bbuild -DCMAKE_BUILD_TYPE=Release .
	@cmake --build build --config Release
	@mv ./build/Release/$(NAME).exe ./$(NAME)_release.exe
	@echo [SUCCESS] $@ compiled successfully without validation layers!

clean:
	@rm -rf ./build/
	@echo [CLEAN] Build files have been removed!

fclean: clean
	@rm -rf $(NAME)_debug.exe $(NAME)_release.exe
	@echo [FCLEAN] Executable files have been fully removed!

re: fclean all

shader :
	@mkdir -p ./spvs
	@for file in ./shaders/*.vert; do \
		glslc $$file -o ./spvs/$$(basename $$file .vert).vert.spv; \
	done
	@for file in ./shaders/*.frag; do \
		glslc $$file -o ./spvs/$$(basename $$file .frag).frag.spv; \
	done
	@echo [SUCCESS] Shaders have been compiled successfully!


.PHONY: all clean fclean re debug release shader