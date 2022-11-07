#pragma once
#include <vector>
#include <memory>
#include "graphics.h"

struct View;
using View_Ptr = std::shared_ptr<View>;


void inlineDirty(View *p, View &parent, std::vector<View_Ptr> &children, IGraphics& graphics);
void blockDirty(View *p, View &parent, std::vector<View_Ptr> &children, IGraphics& graphics);
