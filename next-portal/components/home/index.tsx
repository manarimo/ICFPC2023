import Image from "next/image";

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
  } | null;
};

type Props = {
  problems: ProblemProps[];
};

const Home = ({ problems }: Props) => {
  return (
    <table>
      <thead>
        <tr>
          <th>Problem</th>
          <th>Info</th>
          <th>Image</th>
          <th>Best Solution</th>
        </tr>
      </thead>
      <tbody>
        {problems.map((problem) => (
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
                <li>{problem.bestSolution?.name ?? ""}</li>
                <li>
                  {problem.bestSolution?.score
                    .toString()
                    .replace(/\B(?=(\d{3})+(?!\d))/g, ",") ?? ""}
                </li>
              </ul>
            </td>
          </tr>
        ))}
      </tbody>
    </table>
  );
};

export default Home;
