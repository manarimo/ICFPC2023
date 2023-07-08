import useSWR from "swr";

const useSolutions = () => {
  const url =
    "https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/solutions";
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const data = await response.json();
    return data as {
      solutions: {
        solverName: string;
        problemId: number;
        score: number;
        solutionPath: string;
      }[];
    };
  };
  return useSWR(url, fetcher);
};
export default useSolutions;
