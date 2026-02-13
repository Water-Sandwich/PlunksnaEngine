//
// Created by d on 2/13/26.
//

#ifndef ASSETTYPE_H
#define ASSETTYPE_H

namespace Plunksna {

enum class AssetType
{
    IMAGE = 0,  //jpgs, pngs, etc.
    TEXTURE,    //images with mips and other info
    MESH,       //vertices and indices
    MODEL       //mesh with texture
};

}

#endif //ASSETTYPE_H
