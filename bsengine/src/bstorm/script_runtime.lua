-- NOTE : 全ての引数はr_cpされていない前提
-- 引数からデータを持ち出してはいけない

local function r_copyarray(a)
  local r = {};
  for i=1,#a do r[i] = r_cp(a[i]); end
  return r;
end

local function r_strtodnhstr(s)
  local t = {};
  for i = 1, #s do
    t[i] = string.sub(s, i, i);
  end
  return t;
end

local function r_type_eq(x, y)
  local tx = type(x);
  local ty = type(y);
  if tx == 'table' and ty == 'table' then
    if #x == 0 or #y == 0 then
      return true;
    else
      return r_type_eq(x[1], y[1]);
    end
  else
    return tx == ty;
  end
end

local r_ator = b_ator;

local function r_tonum(x)
  local t = type(x);
  if t == 'number' then return x end
  if t == 'boolean' then return (x and 1 or 0) end
  if t == 'table' then return r_ator(x) end
  if t == 'string' then return c_chartonum(x) end
  if t == 'nil' then return 0 end
  return -1;
end

local function r_toint(x)
  local a = math.modf(r_tonum(x));
  return a;
end

-- throws
function r_checknil(v, name)
  if v == nil then
    if name == nil then
      c_raiseerror("attempt to use empty variable.");
    else
      c_raiseerror("attempt to use empty variable `" .. name .. "'.");
    end
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
    return r_tonum(x) + r_tonum(y);
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
    return r_tonum(x) - r_tonum(y);
  end
end

function r_mul(x,y)
  return r_tonum(x) * r_tonum(y);
end

function r_div(x,y)
  return r_tonum(x) / r_tonum(y);
end

function r_rem(x,y)
  return r_tonum(x) % r_tonum(y);
end

function r_pow(x,y)
  return r_tonum(x) ^ r_tonum(y);
end

function r_not(x)
  return not r_tobool(x);
end

function r_neg(x)
  return -r_tonum(x);
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
    return r_cmp(r_tonum(x), r_tonum(y));
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
    return r_copyarray(x)
  end
  return x;
end

function r_mcat(a, b)
  if type(a) ~= "table" or type(b) ~= "table" then
    c_raiseerror("can't concat non-array values.");
  end

  if not r_type_eq(a, b) then
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

  s = r_toint(s); e = r_toint(e);

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
function r_read(a, idx)
  if type(a) ~= "table" then
    c_raiseerror("attempt to index a non-array value.");
  end

  idx = r_toint(idx);

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

  idx = r_toint(idx);

  if idx >= #a or idx < 0 then
    c_raiseerror("array index out of bounds.");
  end

  idx = idx + 1;

  if not r_type_eq(a[1], v) then
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

    local idx = r_toint(indices[k]);

    if idx >= #a or idx < 0 then
      c_raiseerror("array index out of bounds.");
    end

    idx = idx + 1;

    if k == #indices then
      if not r_type_eq(a[1], v) then
        c_raiseerror("array element type mismatch.");
      end

      a[idx] = r_cp(v);
    end

    a = a[idx];
  end
end

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
  return math.abs(r_tonum(x));
end

b_add = r_add;
b_subtract = r_sub;
b_multiply = r_mul;
b_divide = r_div;
b_remainder = r_rem;
b_power = r_pow
b_not = r_not;
b_negative = r_neg;
b_compare = r_cmp;
b_cat = r_cat;
b_index_ = r_read;
b_slice = r_slice;
b_successor = r_succ;
b_predecessor = r_pred;
b_absolute = r_abs;

-- throws
function b_append(a,x)
  return r_cat(a, {x});
end

-- throws
function b_erase(a, i)
  if(type(a) ~= "table") then
      c_raiseerror("attempt to erase a non-array value.");
  end

  i = r_toint(i);

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

function b_length(a)
  if type(a) ~= "table" then
    return 0;
  else
    return #a;
  end
end

--- math function ---

function b_min(x,y)
  return math.min(r_tonum(x), r_tonum(y));
end

function b_max(x,y)
  return math.max(r_tonum(x), r_tonum(y));
end

function b_log(x)
  return math.log(r_tonum(x));
end

function b_log10(x)
  return math.log10(r_tonum(x));
end

function b_cos(x)
  return math.cos(math.rad(r_tonum(x)));
end

function b_sin(x)
  return math.sin(math.rad(r_tonum(x)));
end

function b_tan(x)
  return math.tan(math.rad(r_tonum(x)));
end

function b_acos(x)
  return math.deg(math.acos(r_tonum(x)));
end

function b_asin(x)
  return math.deg(math.asin(r_tonum(x)));
end

function b_atan(x)
  return math.deg(math.atan(r_tonum(x)));
end

function b_atan2(y,x)
  return math.deg(math.atan2(r_tonum(y), r_tonum(x)));
end

math.randomseed(os.time()); -- FUTURE : Systemから与えられたシードで初期化

function b_rand(min, max)
  min = r_tonum(min);
  return math.random() * (r_tonum(max) - min) + min;
end

function b_round(x)
  return math.floor(r_tonum(x) + 0.5);
end

b_truncate = r_toint;

b_trunc = r_toint;

function b_floor(x)
  return math.floor(r_tonum(x));
end

function r_ceil(x)
  return math.ceil(r_tonum(x));
end

b_ceil = r_ceil;

function b_modc(x, y)
  return math.fmod(r_tonum(x), r_tonum(y));
end

function b_IntToString(n)
  return r_strtodnhstr(string.format("%d", r_toint(n)));
end

b_itoa = b_IntToString

function b_rtoa(n)
  return r_strtodnhstr(string.format("%f", r_tonum(n)));
end

script_event_type = -1;
script_event_args = {};

function b_GetEventType()
  return script_event_type;
end

function b_GetEventArgument(idx)
  -- 小数は切り捨てる
  idx = r_toint(idx) + 1;
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