-- NOTE : 全ての引数はr_cpされていない前提
-- 引数からデータを持ち出してはいけない

-- r_: runtime
-- rl_: runtime local
-- rb_: runtime builtin

local function rl_copyarray(a)
  local r = {};
  for i=1,#a do r[i] = r_cp(a[i]); end
  return r;
end

local function rl_strtodnhstr(s)
  local t = {};
  for i = 1, #s do
    t[i] = string.sub(s, i, i);
  end
  return t;
end

local function rl_type_eq(x, y)
  local tx = type(x);
  local ty = type(y);
  if tx == 'table' and ty == 'table' then
    if #x == 0 or #y == 0 then
      return true;
    else
      return rl_type_eq(x[1], y[1]);
    end
  else
    return tx == ty;
  end
end

local function rl_tonum(x)
  local t = type(x);
  if t == 'number' then return x end
  if t == 'boolean' then return (x and 1 or 0) end
  if t == 'table' then return c_ator(x) end
  if t == 'string' then return c_chartonum(x) end
  if t == 'nil' then return 0 end
  return -1;
end

local function rl_toint(x)
  local a = math.modf(rl_tonum(x));
  return a;
end

-- throws
function r_nc(v, name) -- nilcheck
  if v == nil then
    c_raiseerror("attempt to use empty variable `" .. name .. "'.");
  else
    return v;
  end
end

function r_tobool(x)
  local t = type(x);
  if(t == 'boolean') then return x end
  if(t == 'number') then return x ~= 0 end
  if(t == 'table') then return #x ~= 0 end
  if(t == 'string') then return c_chartonum(x) ~= 0 end
  if(t == 'nil') then return false end
  return false
end

-- throws
function r_add(x, y)
  if type(x) == "table" then
    if type(y) ~= "table" then
      c_raiseerror("can't apply '+' to diffrent type values.");
    end
    if #x ~= #y then
      c_raiseerror("can't apply '+' to diffrent length arrays.");
    end
    local z = {};
    for i = 1, #x do
      z[i] = r_add(x[i], y[i]);
    end
    return z;
  else
    return rl_tonum(x) + rl_tonum(y);
  end
end

-- throws
function r_sub(x, y)
  if type(x) == "table" then
    if type(y) ~= "table" then
      c_raiseerror("can't compare diffrent type values.");
    end
    if #x ~= #y then
      c_raiseerror("can't apply '-' to diffrent length arrays.");
    end
    local z = {};
    for i = 1, #x do
      z[i] = r_sub(x[i], y[i]);
    end
    return z;
  else
    return rl_tonum(x) - rl_tonum(y);
  end
end

function r_mul(x,y)
  return rl_tonum(x) * rl_tonum(y);
end

function r_div(x,y)
  return rl_tonum(x) / rl_tonum(y);
end

function r_rem(x,y)
  return rl_tonum(x) % rl_tonum(y);
end

function r_pow(x,y)
  return rl_tonum(x) ^ rl_tonum(y);
end

function r_not(x)
  return not r_tobool(x);
end

function r_neg(x)
  return -rl_tonum(x);
end

-- throws
local function r_cmp(x, y)
  if type(x) ~= type(y) then
    c_raiseerror("can't compare diffrent type values.");
  end

  if type(x) == "number" then
    if x < y then
      return -1;
    elseif x > y then
      return 1;
    else
      return 0;
    end
  end

  if type(x) == "string" then
    return r_cmp(c_chartonum(x), c_chartonum(y));
  end

  if type(x) == "boolean" then
    return r_cmp(rl_tonum(x), rl_tonum(y));
  end

  if type(x) == "table" then
    for i = 1, math.min(#x, #y) do
      local r = r_cmp(x[i], y[i]);
      if r ~= 0 then return r end
    end
    return r_cmp(#x, #y);
  end

  return 0;
end

function r_eq(x, y) return r_cmp(x, y) == 0; end
function r_ne(x, y) return not r_eq(x, y); end
function r_lt(x, y) return r_cmp(x, y) < 0; end
function r_le(x, y) return r_cmp(x, y) <= 0; end
function r_gt(x, y) return not r_le(x, y); end
function r_ge(x, y) return not r_lt(x, y); end

function r_and(x, f)
  if r_tobool(x) then return f() end
  return x;
end

function r_or(x, f)
  if r_tobool(x) then return x end
  return f();
end

function r_cp(x)
  if type(x) == "table" then
    return rl_copyarray(x)
  end
  return x;
end

function r_mcat(a, b)
  if type(a) ~= "table" or type(b) ~= "table" then
    c_raiseerror("can't concat non-array values.");
  end

  if not rl_type_eq(a, b) then
    c_raiseerror("can't concat diffrent type values.");
  end

  local j = #a + 1;
  for i = 1, #b do a[j] = r_cp(b[i]); j = j + 1; end
  return a;
end

-- throws
function r_cat(a, b)
  return r_mcat(r_cp(a), b);
end

-- throws
function r_slice(a, s, e)
  if type(a) ~= "table" then
    c_raiseerror("can't slice non-array values.");
  end

  s = rl_toint(s); e = rl_toint(e);

  if s > e  or s > #a or e > #a or s < 0 or e < 0 then
    c_raiseerror("array index out of bounds.");
  end

  s = s + 1; e = e + 1;

  local r = {};
  local i = 1;
  for j = s, e-1 do
    r[i] = r_cp(a[j]);
    i = i + 1;
  end
  return r;
end

-- throws
function r_arr(a)
  for i=2,#a do
    if not rl_type_eq(a[1], a[i]) then
        -- c_raiseerror("can' create an array in which elements  different type.");
    end
  end
  return a;
end

-- throws
function r_read(a, idx)
  if type(a) ~= "table" then
    c_raiseerror("attempt to index a non-array value.");
  end

  idx = rl_toint(idx);

  if idx >= #a or idx < 0 then
    c_raiseerror("array index out of bounds.");
  end

  idx = idx + 1;

  return r_cp(a[idx]);
end

-- throws
function r_write1(a, idx, v)
  if type(a) ~= "table" then
    c_raiseerror("attempt to index a non-array value.");
  end

  idx = rl_toint(idx);

  if idx >= #a or idx < 0 then
    c_raiseerror("array index out of bounds.");
  end

  idx = idx + 1;

  if not rl_type_eq(a[1], v) then
    c_raiseerror("array element type mismatch.");
  end

  a[idx] = r_cp(v);
end

-- throws
function r_write(a, indices, v)
  for k=1, #indices do
    if type(a) ~= "table" then
      c_raiseerror("attempt to index a non-array value.");
    end

    local idx = rl_toint(indices[k]);

    if idx >= #a or idx < 0 then
      c_raiseerror("array index out of bounds.");
    end

    idx = idx + 1;

    if k == #indices then
      if not rl_type_eq(a[1], v) then
        c_raiseerror("array element type mismatch.");
      end

      a[idx] = r_cp(v);
    end

    a = a[idx];
  end
end

-- throws
function r_succ(x)
  if type(x) == "number" then
    return x + 1;
  elseif type(x) == "string" then
    return c_succchar(x);
  elseif type(x) == "boolean" then
    return true;
  elseif type(x) == "table" then
    c_raiseerror("can't apply successor for array.");
  end
  return x;
end

-- throws
function r_pred(x)
  if type(x) == "number" then
    return x - 1;
  elseif type(x) == "string" then
    return c_predchar(x);
  elseif type(x) == "boolean" then
    return false;
  elseif type(x) == "table" then
    c_raiseerror("can't apply predecessor for array.");
  end
  return x;
end

function r_abs(x)
  return math.abs(rl_tonum(x));
end

function r_yield()
  return coroutine.yield();
end

rb_add = r_add;
rb_subtract = r_sub;
rb_multiply = r_mul;
rb_divide = r_div;
rb_remainder = r_rem;
rb_power = r_pow
rb_not = r_not;
rb_negative = r_neg;
rb_compare = r_cmp;
rb_concatenate = r_cat;
rb_index_ = r_read;
rb_slice = r_slice;
rb_successor = r_succ;
rb_predecessor = r_pred;
rb_absolute = r_abs;

-- throws
function rb_append(a,x)
  return r_cat(a, {x});
end

-- throws
function rb_erase(a, i)
  if(type(a) ~= "table") then
      c_raiseerror("attempt to erase a non-array value.");
  end

  i = rl_toint(i);

  if i >= #a or i < 0 then
    c_raiseerror("array index out of bounds.");
  end

  i = i + 1;

  local r = {};
  for j = 1, #a do
    if j ~= i then table.insert(r, r_cp(a[j])); end
  end
  return r;
end

function rb_length(a)
  if type(a) ~= "table" then
    return 0;
  else
    return #a;
  end
end

--- math function ---

function rb_min(x,y)
  return math.min(rl_tonum(x), rl_tonum(y));
end

function rb_max(x,y)
  return math.max(rl_tonum(x), rl_tonum(y));
end

function rb_log(x)
  return math.log(rl_tonum(x));
end

function rb_log10(x)
  return math.log10(rl_tonum(x));
end

function rb_cos(x)
  return math.cos(math.rad(rl_tonum(x)));
end

function rb_sin(x)
  return math.sin(math.rad(rl_tonum(x)));
end

function rb_tan(x)
  return math.tan(math.rad(rl_tonum(x)));
end

function rb_acos(x)
  return math.deg(math.acos(rl_tonum(x)));
end

function rb_asin(x)
  return math.deg(math.asin(rl_tonum(x)));
end

function rb_atan(x)
  return math.deg(math.atan(rl_tonum(x)));
end

function rb_atan2(y,x)
  return math.deg(math.atan2(rl_tonum(y), rl_tonum(x)));
end

math.randomseed(os.time()); -- FUTURE : Systemから与えられたシードで初期化

function rb_rand(min, max)
  min = rl_tonum(min);
  return math.random() * (rl_tonum(max) - min) + min;
end

function rb_round(x)
  return math.floor(rl_tonum(x) + 0.5);
end

rb_truncate = rl_toint;

rb_trunc = rl_toint;

function rb_floor(x)
  return math.floor(rl_tonum(x));
end

function r_ceil(x)
  return math.ceil(rl_tonum(x));
end

rb_ceil = r_ceil;

function rb_modc(x, y)
  return math.fmod(rl_tonum(x), rl_tonum(y));
end

function rb_IntToString(n)
  return rl_strtodnhstr(string.format("%d", rl_toint(n)));
end

rb_itoa = rb_IntToString

function rb_rtoa(n)
  return rl_strtodnhstr(string.format("%f", rl_tonum(n)));
end

script_event_type = -1;
script_event_args = {};

function rb_GetEventType()
  return script_event_type;
end

function rb_GetEventArgument(idx)
  -- 小数は切り捨てる
  idx = rl_toint(idx) + 1;
  if idx < 1 or idx > #script_event_args then
    c_raiseerror("event arguments array index out of bounds.");
  end
  return r_cp(script_event_args[idx]);
end

--- task ---

-- task state
local TASK_RUNNING = 0;
local TASK_SUSPENDED = 1;

local Task = {}; Task.__index = Task;

function Task:create()
  local t = {func = nil, args = nil, state = TASK_SUSPENDED};
  t.step = coroutine.wrap(function()
    while true do
      if t.args == nil then
        t.func();
      else
        t.func(unpack(t.args));
      end
      t.func = nil; t.args = nil;
      t.state = TASK_SUSPENDED;
      coroutine.yield();
    end
  end);
  return setmetatable(t, Task);
end

function Task:resume()
  if self.state == TASK_SUSPENDED then return; end
  return self.step();
end

function Task:set_func(f, args)
  -- assert(self.state == TASK_SUSPENDED);
  self.func = f;
  self.args = args;
  self.state = TASK_RUNNING;
end

-- task manager
local running_sub_tasks, added_sub_tasks, pooled_sub_tasks = {}, {}, {};

local function add_sub_task(f, args)
  local t = table.remove(pooled_sub_tasks);
  if t == nil then
    t = Task:create();
  end
  t:set_func(f, args);
  t:resume();
  if t.state == TASK_SUSPENDED then
    pooled_sub_tasks[#pooled_sub_tasks+1] = t;
  else
    added_sub_tasks[#added_sub_tasks+1] = t;
  end
end

local function resume_all_sub_task()
  running_sub_tasks, added_sub_tasks = added_sub_tasks, running_sub_tasks;
  for i,t in ipairs(running_sub_tasks) do
    t:resume();
    if t.state == TASK_SUSPENDED then
      pooled_sub_tasks[#pooled_sub_tasks+1] = t;
    else
      added_sub_tasks[#added_sub_tasks+1] = t;
    end
    running_sub_tasks[i] = nil;
  end
end

local main_task = Task:create();

-- run builtin routine
function r_run(f)
  main_task:set_func(f, nil);
  while true do
    main_task:resume();
    if main_task.state == TASK_SUSPENDED then
      break;
    end
    resume_all_sub_task();
  end
end

-- fork sub task
function r_fork(func, args) return add_sub_task(func, args); end
function r_fork0(func) return add_sub_task(func); end
function r_fork1(func, arg1) return add_sub_task(func, {arg1}); end
function r_fork2(func, arg1, arg2) return add_sub_task(func, {arg1, arg2}); end
function r_fork3(func, arg1, arg2, arg3) return add_sub_task(func, {arg1, arg2, arg3}); end
function r_fork4(func, arg1, arg2, arg3, arg4) return add_sub_task(func, {arg1, arg2, arg3, arg4}); end
function r_fork5(func, arg1, arg2, arg3, arg4, arg5) return add_sub_task(func, {arg1, arg2, arg3, arg4, arg5}); end
function r_fork6(func, arg1, arg2, arg3, arg4, arg5, arg6) return add_sub_task(func, {arg1, arg2, arg3, arg4, arg5, arg6}); end
function r_fork7(func, arg1, arg2, arg3, arg4, arg5, arg6, arg7) return add_sub_task(func, {arg1, arg2, arg3, arg4, arg5, arg6, arg7}); end