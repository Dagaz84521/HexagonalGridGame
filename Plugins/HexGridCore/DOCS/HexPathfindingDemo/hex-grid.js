(function () {
  "use strict";

  const SQRT3 = Math.sqrt(3);
  const DIRECTIONS = [
    { name: "Right", q: 1, r: 0 },
    { name: "TopRight", q: 1, r: -1 },
    { name: "TopLeft", q: 0, r: -1 },
    { name: "Left", q: -1, r: 0 },
    { name: "BottomLeft", q: -1, r: 1 },
    { name: "BottomRight", q: 0, r: 1 }
  ];

  function key(coord) {
    return `${coord.q},${coord.r}`;
  }

  function parseKey(value) {
    const parts = value.split(",").map(Number);
    return { q: parts[0], r: parts[1] };
  }

  function getS(coord) {
    return -coord.q - coord.r;
  }

  function add(a, b) {
    return { q: a.q + b.q, r: a.r + b.r };
  }

  function getAllNeighbours(coord) {
    return DIRECTIONS.map((direction) => add(coord, direction));
  }

  function distance(a, b) {
    const dq = a.q - b.q;
    const dr = a.r - b.r;
    const ds = getS(a) - getS(b);
    return Math.max(Math.abs(dq), Math.abs(dr), Math.abs(ds));
  }

  function hexToPixel(coord, size) {
    return {
      x: size * SQRT3 * (coord.q + coord.r / 2),
      y: size * 1.5 * coord.r
    };
  }

  function pixelToHex(point, size) {
    const q = (SQRT3 / 3 * point.x - 1 / 3 * point.y) / size;
    const r = (2 / 3 * point.y) / size;
    return cubeRound(q, r, -q - r);
  }

  function cubeRound(q, r, s) {
    let rq = Math.round(q);
    let rr = Math.round(r);
    let rs = Math.round(s);
    const qDiff = Math.abs(rq - q);
    const rDiff = Math.abs(rr - r);
    const sDiff = Math.abs(rs - s);

    if (qDiff > rDiff && qDiff > sDiff) {
      rq = -rr - rs;
    } else if (rDiff > sDiff) {
      rr = -rq - rs;
    }

    return { q: rq, r: rr };
  }

  function buildHexagon(radius) {
    const result = [];
    for (let q = -radius; q <= radius; q += 1) {
      const r1 = Math.max(-radius, -q - radius);
      const r2 = Math.min(radius, -q + radius);
      for (let r = r1; r <= r2; r += 1) {
        result.push({ q, r });
      }
    }
    return result;
  }

  function createAStar(gridKeys, blockedKeys, startKey, goalKey) {
    const open = [{ key: startKey, g: 0, f: distance(parseKey(startKey), parseKey(goalKey)) }];
    const closed = new Set();
    const cameFrom = new Map();
    const gCost = new Map([[startKey, 0]]);
    const fCost = new Map([[startKey, distance(parseKey(startKey), parseKey(goalKey))]]);
    let phase = "start";
    let finished = false;
    let found = false;
    let currentKey = null;
    let currentNode = null;
    let currentNeighborKey = null;
    let currentTentativeG = null;
    let neighborList = [];
    let neighborIndex = 0;
    let path = [];
    let activeLine = 1;
    let message = "准备开始：起点进入 open。";

    function reconstruct(goal) {
      const result = [];
      let cursor = goal;
      while (cameFrom.has(cursor)) {
        result.push(cursor);
        cursor = cameFrom.get(cursor);
      }
      return result.reverse();
    }

    function step() {
      if (finished) {
        return snapshot();
      }

      if (phase === "start") {
        activeLine = 2;
        message = "初始化 open、closed、GCost。";
        phase = "loop";
        return snapshot();
      }

      if (phase === "loop") {
        activeLine = 3;
        currentNeighborKey = null;
        currentTentativeG = null;
        if (!gridKeys.has(startKey) || !gridKeys.has(goalKey) || blockedKeys.has(startKey) || blockedKeys.has(goalKey)) {
          finished = true;
          activeLine = 13;
          message = "起点或终点无效，无法寻路。";
        } else if (startKey === goalKey) {
          finished = true;
          found = true;
          activeLine = 5;
          message = "起点就是终点。";
        } else if (open.length === 0) {
          finished = true;
          activeLine = 13;
          message = "open 已空，没有找到路径。";
        } else {
          phase = "select";
          message = "open 非空，继续搜索。";
        }
        return snapshot();
      }

      if (phase === "select") {
        activeLine = 4;
        open.sort((a, b) => a.f - b.f || a.g - b.g);
        currentNode = null;

        while (open.length > 0) {
          const node = open.shift();
          if (closed.has(node.key)) {
            continue;
          }
          if (gCost.get(node.key) !== node.g) {
            continue;
          }
          currentNode = node;
          currentKey = node.key;
          break;
        }

        if (currentNode) {
          phase = "goal";
          message = `选择 F 最小的格子 ${currentKey}。`;
        } else {
          phase = "loop";
          message = "open 中没有可用节点，回到循环检查。";
        }
        return snapshot();
      }

      if (phase === "goal") {
        activeLine = 5;
        if (currentKey === goalKey) {
          finished = true;
          found = true;
          path = reconstruct(currentKey);
          message = "当前格就是终点，回溯 cameFrom 得到路径。";
        } else {
          phase = "close";
          message = `${currentKey} 不是终点，继续扩展邻居。`;
        }
        return snapshot();
      }

      if (phase === "close") {
        activeLine = 6;
        closed.add(currentKey);
        neighborList = getAllNeighbours(parseKey(currentKey));
        neighborIndex = 0;
        currentNeighborKey = null;
        phase = "neighbor";
        message = `把 ${currentKey} 加入 closed。`;
        return snapshot();
      }

      if (phase === "neighbor") {
        activeLine = 7;
        currentTentativeG = null;
        if (neighborIndex >= neighborList.length) {
          phase = "loop";
          currentNeighborKey = null;
          message = `${currentKey} 的六个邻居检查完毕。`;
          return snapshot();
        }

        currentNeighborKey = key(neighborList[neighborIndex]);
        neighborIndex += 1;
        phase = "filter";
        message = `检查邻居 ${currentNeighborKey}。`;
        return snapshot();
      }

      if (phase === "filter") {
        activeLine = 8;
        if (!gridKeys.has(currentNeighborKey)) {
          phase = "neighbor";
          message = `${currentNeighborKey} 不在棋盘内，跳过。`;
        } else if (blockedKeys.has(currentNeighborKey)) {
          phase = "neighbor";
          message = `${currentNeighborKey} 是阻挡格，跳过。`;
        } else if (closed.has(currentNeighborKey)) {
          phase = "neighbor";
          message = `${currentNeighborKey} 已在 closed 中，跳过。`;
        } else {
          phase = "cost";
          message = `${currentNeighborKey} 可以继续评估。`;
        }
        return snapshot();
      }

      if (phase === "cost") {
        activeLine = 9;
        currentTentativeG = gCost.get(currentKey) + 1;
        phase = "improve";
        message = `计算 tentativeG = G[${currentKey}] + 1 = ${currentTentativeG}。`;
        return snapshot();
      }

      if (phase === "improve") {
        activeLine = 10;
        const existingG = gCost.get(currentNeighborKey);
        if (existingG !== undefined && currentTentativeG >= existingG) {
          phase = "neighbor";
          message = `${currentNeighborKey} 已有更短或相同路径，跳过。`;
        } else {
          phase = "record";
          message = `${currentNeighborKey} 得到更好的路径。`;
        }
        return snapshot();
      }

      if (phase === "record") {
        activeLine = 11;
        cameFrom.set(currentNeighborKey, currentKey);
        gCost.set(currentNeighborKey, currentTentativeG);
        phase = "push";
        message = `记录 cameFrom[${currentNeighborKey}] = ${currentKey}。`;
        return snapshot();
      }

      if (phase === "push") {
        activeLine = 12;
        const neighbour = parseKey(currentNeighborKey);
        const h = distance(neighbour, parseKey(goalKey));
        const f = currentTentativeG + h;
        fCost.set(currentNeighborKey, f);
        open.push({ key: currentNeighborKey, g: currentTentativeG, f });
        phase = "neighbor";
        message = `计算 H=${h}，F=${f}，把 ${currentNeighborKey} 加入 open。`;
        return snapshot();
      }

      finished = true;
      activeLine = 13;
      message = "没有找到路径。";
      return snapshot();
    }

    function snapshot() {
      return {
        open: new Set(open.map((node) => node.key)),
        closed: new Set(closed),
        currentKey,
        currentNeighborKey,
        path: path.slice(),
        finished,
        found,
        activeLine,
        message,
        cameFrom: new Map(cameFrom),
        gCost: new Map(gCost),
        fCost: new Map(fCost)
      };
    }

    return { step, snapshot };
  }

  window.HexGridDemo = {
    DIRECTIONS,
    key,
    parseKey,
    getS,
    add,
    getAllNeighbours,
    distance,
    hexToPixel,
    pixelToHex,
    buildHexagon,
    createAStar
  };
})();
