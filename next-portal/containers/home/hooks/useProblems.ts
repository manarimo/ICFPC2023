import useSWR from "swr";

const useProblems = () => {
  const url =
    "https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/problems";
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const data = await response.json();
    return data as {
      problems: {
        problemId: number;
        roomHeight: number;
        roomWidth: number;
        numAttendees: number;
        numMusicians: number;
      }[];
    };
  };
  return useSWR(url, fetcher);
};
export default useProblems;
