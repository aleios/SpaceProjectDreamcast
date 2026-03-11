state = 0

function init()
    local x, y = enemy:pos()
    enemy:move_to(x, y + math.random(40, 350))
    state = 0
end

function collided_boundary(direction)
    if state == 0 then
        if direction == Direction.Left then
            enemy:move_sine(0, 0.1, 0.5, 250.0)
            enemy:activate_weapon(0)
        elseif direction == Direction.Right then
            enemy:move_sine(180, 0.1, 0.5, 250.0)
            enemy:deactivate_weapon(0)
        end
    end
end

function target_arrive()
    if state == 0 then
        enemy:move_direction(0, 0.1)
    elseif state == 1 then
        enemy:move_to_player(0.15)
    end
end

function damaged(damage)
    if enemy:health() < 2 and state == 0 then
        state = 1
        enemy:move_to_player(0.15)
    end
end

return {
    handlers = {
        init = init,
        on_collide_boundary = collided_boundary,
        on_target_arrive = target_arrive,
        on_damage = damaged
    }
}
