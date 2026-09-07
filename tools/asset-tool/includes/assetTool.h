
#ifndef ASSET_TOOLS_TOOL_H
#define ASSET_TOOLS_TOOL_H

#include "common.h"

namespace cyclonite::tools 
{
class AssetTool
{
public:
    static void doCommand(AssetToolCommand& command);
};
}

#endif // ASSET_TOOLS_TOOL_H
