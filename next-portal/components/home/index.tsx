import Image from "next/image";
import { useMemo, useState } from "react";
import { SolutionHistoryItem } from "../../containers/home/hooks";

type ProblemProps = {
  problemId: number;
  roomWidth: number;
  roomHeight: number;
  numMusicians: number;
  numAttendees: number;
  bestSolution: {
    name: string;
    score: number;
    solutionPath: string;
    tag: string;
  } | null;
  history: SolutionHistoryItem[];
};

type Props = {
  problems: ProblemProps[];
};

type SortKey = 'id' | 'score';

const Home = ({ problems: propProblems }: Props) => {
  const [sortKey, setSortKey] = useState('id' as SortKey);
  const problems = useMemo(() => {
    if (sortKey == 'score') {
      const tmp = [...propProblems];
      tmp.sort((a, b) => (b.bestSolution?.score || 0) - (a.bestSolution?.score || 0));
      return tmp;
    }
    return propProblems;
  }, [propProblems, sortKey]);
  const pointSum = useMemo(() => {
    let sum = 0;
    problems.forEach((problem) => {
      if (problem.bestSolution) {
        sum += problem.bestSolution.score;
      }
    });
    return sum;
  }, [problems]);
  const [prevPointSum, setPrevPointSum] = useState(pointSum);
  if (prevPointSum !== pointSum) {
    setPrevPointSum(pointSum);
    console.log(`point update detected: ${prevPointSum} -> ${pointSum}`);
    const audio = new Audio('/inception.aac');
    audio.play();
  }
  
  return (
    <>
    <h2>Total score: {formatNumber(pointSum)}</h2>
    <table>
      <thead>
        <tr>
          <th>Problem</th>
          <th>Info</th>
          <th>Image</th>
          <th>
            <div>Best Solution</div>
            <input type="checkbox" checked={sortKey == 'score'} onChange={(e) => setSortKey(e.currentTarget.checked ? 'score' : 'id')}></input>Sort by score
          </th>
          <th>Best Solution Image</th>
          <th>Score History</th>
        </tr>
      </thead>
      <tbody>
        {problems.filter((p) => p.bestSolution != undefined).map((problem) => (
          <tr key={problem.problemId}>
            <td
              style={{
                border: "1px solid black",
                padding: "5px",
              }}
            >
              {problem.problemId}
            </td>
            <td
              style={{
                border: "1px solid black",
                padding: "5px",
              }}
            >
              <ul>
                <li>
                  {problem.roomWidth}x{problem.roomHeight}
                </li>
                <li>
                  Attendees:{" "}
                  {problem.numAttendees
                    .toString()
                    .replace(/\B(?=(\d{3})+(?!\d))/g, ",") ?? ""}
                </li>
                <li>
                  Musician:{" "}
                  {problem.numMusicians
                    .toString()
                    .replace(/\B(?=(\d{3})+(?!\d))/g, ",") ?? ""}
                </li>
              </ul>
            </td>
            <td
              style={{
                border: "1px solid black",
                padding: "5px",
              }}
            >
              <Image
                width={400}
                height={200}
                src={`/public/problem_images/${problem.problemId}.svg`}
                alt=""
              />
            </td>
            <td
              style={{
                border: "1px solid black",
                padding: "5px",
              }}
            >
              <ul>
                <li>{problem.bestSolution?.name ?? ""} ({problem.bestSolution?.tag})</li>
                <li>
                  {problem.bestSolution?.score
                    .toString()
                    .replace(/\B(?=(\d{3})+(?!\d))/g, ",") ?? ""}
                </li>
                {problem.bestSolution ? (
                  <li>{`${
                    Math.floor(
                      (problem.bestSolution.score / pointSum) * 1000000
                    ) / 10000
                  }%`}</li>
                ) : null}
              </ul>
            </td>
            <td
              style={{
                border: "1px solid black",
                padding: "5px",
              }}
            >
              {problem.bestSolution? (
                <Image
                  width={400}
                  height={200}
                  src={`/solutions/${problem.bestSolution.name}/images/${problem.problemId}.svg`}
                  alt=""
                />
              ) : null}
            </td>
            <td
              style={{
                border: "1px solid black",
                padding: "5px",
              }}
            >
              <ul>
                {problem.history.map((item) => (
                  <li>{formatNumber(item.score)} <span style={{ fontSize: '10px' }}>(+{formatNumber(item.diffAbs || 0)}, +{formatPercent(item.diffPct || 0)}%)</span></li>
                ))}
              </ul>
            </td>
          </tr>
        ))}
      </tbody>
    </table>
    </>
  );
};

function formatNumber(value: number): string {
  return value.toString()
    .replace(/\B(?=(\d{3})+(?!\d))/g, ",") ?? "";
}

function formatPercent(value: number): string {
  const dec = Math.floor(value);
  const frac = Math.floor((value - dec) * 100).toString().padStart(2, '0');
  return `${dec}.${frac}`;
}

export default Home;
