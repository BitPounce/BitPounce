class Player : IScript
{
    float time;
    Entity@ ent;          // template is fine
    
    void OnCreate(Entity@ e)
    {
        @ent = e; // template constructor
        time = 0.0f;
        print("Player created!");
        
        e.GetTransform().Translation = vec3(0.0f, 0.0f, 0.0f);
        e.GetTransform().Scale = vec3(1.0f, 1.0f, 1.0f);
    }
    
    void OnUpdate(float dt)
    {

        time += dt;
        
        float radius = 2.0f;
        float speed = 1.0f;
        ent.GetTransform().Translation.x = cos(time * speed) * radius;
        ent.GetTransform().Translation.y = sin(time * speed) * radius;
        ent.GetTransform().Rotation.z += dt;
        
        float scale = 0.5f + 0.5f * sin(time * 2.0f);
        ent.GetTransform().Scale = vec3(scale, scale, 1.0f);
    }
    
    void OnDestroy()
    {
        print("Player destroyed");
    }
}