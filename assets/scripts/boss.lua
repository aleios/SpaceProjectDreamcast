function init()
    local x, y = enemy:pos()
    enemy:move_to(x, Constants.ScreenHeight/2.0)
    enemy:activate_weapon(0)
end

return {
    handlers = {
        init = init
    }
}
