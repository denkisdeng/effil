require "bootstrap-tests"

local effil = effil

test.thread_interrupt.tear_down = default_tear_down

test.thread_interrupt.channel_wait = function()
    local state = effil.table({ stop = false })

    local thr = effil.thread(function()
        effil.thread(function()
            while not state.stop do end
        end)():wait()
    end)()

    effil.sleep(1) -- let thread starts

    local start_time = os.time()
    thr:cancel()
    test.almost_equal(os.time(), start_time, 5)
    state.stop = true
end
