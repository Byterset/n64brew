import bpy
import re
import math


"""
exports object transforms for every frame of an animation (interpolated from keyframes)
"""

# we scale the models up by this much to avoid n64 fixed point precision issues
N64_SCALE_FACTOR = 30

modelname = "goose"
animation_name = "walk"
filename = modelname+animation_name+"anim"

include_guard = filename.upper()+'_H'

out = """
#ifndef %s
#define %s 1
#include "animationframe.h"
#include "%smeshlist.h"

""" % (include_guard,include_guard,modelname)


out += """
AnimationFrame %s_data[] = {
""" % (filename)

scene = bpy.context.scene
frame_current = scene.frame_current

for frame in range(scene.frame_start, scene.frame_end + 1):
    scene.frame_set(frame)
    for obj in scene.objects:
        if modelname in obj.name.lower() and obj in bpy.context.visible_objects:
            mat = obj.matrix_world
            pos = mat.translation.xzy
            rot = mat.to_euler()

            out += "{"
            out += "%d, " % (frame-scene.frame_start)

            out += "%s_mesh, " % (re.sub(r'[.].*?$', '', obj.name))

            out += "{%f, %f, %f}, " % (pos.x*N64_SCALE_FACTOR, pos.z*N64_SCALE_FACTOR, -(pos.y*N64_SCALE_FACTOR)) # y axis is inverted
            out += "{%f, %f, %f}, " % (math.degrees(rot.x), math.degrees(rot.z), math.degrees(rot.y))
            out += "},\n" 

scene.frame_set(frame_current)

out += """
};
""" 

out += """
#define %s_FRAME_COUNT %d
""" % (filename.upper(),  scene.frame_end - scene.frame_start + 1)

out += """
#endif /* %s */
""" % (include_guard)

outfile = open(filename+'.h', 'w')
outfile.write(out)
outfile.close()
  