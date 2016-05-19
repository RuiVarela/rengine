// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SCENE_H__
#define __RENGINE_SCENE_H__

namespace rengine
{
    class Scene
    {
        public:
			Scene() {};
            virtual ~Scene() {};

            virtual void init() = 0;
            virtual void shutdown() = 0;
            virtual void update() = 0;
            virtual void render() = 0;
        private:
    };

}// end of namespace

#endif // __RENGINE__SCENE_H__
